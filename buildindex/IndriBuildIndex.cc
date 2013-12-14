/*==========================================================================
 * Copyright (c) 2004 University of Massachusetts.  All Rights Reserved.
 *
 * Use of the Lemur Toolkit for Language Modeling and Information Retrieval
 * is subject to the terms of the software license set forth in the LICENSE
 * file included with this software, and also available at
 * http://www.lemurproject.org/license.html
 *
 *==========================================================================
*/

#include <algorithm>
#include <cstring>
#include <string>
#include <cctype>

#include "indri/Parameters.hpp"
#include "indri/IndexEnvironment.hpp"
#include <time.h>
#include "indri/Path.hpp"
#include "indri/ConflationPattern.hpp"
#include "lemur/Exception.hpp"
#include "indri/FileTreeIterator.hpp"
#include <vector>
#include <map>
#include "indri/IndriTimer.hpp"

#include "indri/QueryEnvironment.hpp"
#include "indri/Thread.hpp"
#include "indri/SequentialWriteBuffer.hpp"

#include <math.h>

#include "indri/Repository.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/DirectoryIterator.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/Path.hpp"

#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "ThriftDocumentExtractor.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"

// Recover a repository that crashed during build to be consistent with
// its latest checkpoint. If it can't be recovered, create an empty one.
static bool _recoverRepository(const std::string &path) {
  indri::collection::Repository repo;
  try {
    repo.open(path);
  } catch (lemur::api::Exception &ex) {
    // failed to open, can't fix it, recreate.
    return false;
  }
  
  // count up the documents that made it to disk
  indri::collection::Repository::index_state indexes = repo.indexes();
  INT64 total = 0;
  for( size_t i = 0; i < indexes->size(); i++ ) {
    indri::thread::ScopedLock lock( (*indexes)[i]->statisticsLock() );
    total += (*indexes)[i]->documentCount();
  }
  total -= repo.deletedList().deletedCount();

  // identify the docids that are in the collection but not in a disk index
  indri::collection::CompressedCollection *col = repo.collection();
  indri::index::DeletedDocumentList del;
  bool marked = false;
  int numMarked = 0;
  for (int i = (int)total + 1; col->exists(i); i++) {
      del.markDeleted(i);
      marked = true;
      numMarked++;
  }
  // compact to delete the data associated with the unindexed docids.
  if (marked) {
    try {
      std::cerr << "Reovering Repository: " << path << "\nDeleting " 
                << numMarked << " uncommitted documents." << std::endl;
      col->compact(del);
      // check for any partial disk indexes (crash during write)
      // and remove them
      std::string indexPath = indri::file::Path::combine( path, "index" );
      indri::file::DirectoryIterator idirs( indexPath );
      while (! (idirs == indri::file::DirectoryIterator::end())) {
        // iterate over the subdirectories, removing any that don't have a 
        // manifest file.
        std::string current = *idirs;
        std::string manifest = indri::file::Path::combine(current, "manifest");
        if (!indri::file::Path::exists(manifest)) {
          std::cerr << "Removing corrupted index directory: " << current 
                    << std::endl;
          indri::file::Path::remove(current);
          }
        idirs++;
      }
    } catch (lemur::api::Exception &e) {
      // no recovery possible here...
      LEMUR_ABORT(e);
    }
  }
  repo.close();
  // successfully opened and closed
  return true;
}

static indri::utility::IndriTimer g_timer;

static void buildindex_start_time() {
  g_timer.start();
}

static void buildindex_print_status( const char* status, int count ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << status << count << "\r";
}

static void buildindex_print_status( const char* status, int count, const char* status2, INT64 count2 ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << status << count << status2 << count2 << "\r";
}

static void buildindex_flush_status() {
  std::cout.flush();
}

static void buildindex_print_event( const char* event ) {
  g_timer.printElapsedSeconds(std::cout);
  std::cout << ": " << event << std::endl;
}

static void buildindex_print_event( std::string event ) {
  buildindex_print_event( event.c_str() );
}

class StatusMonitor : public indri::api::IndexStatus {
  void operator() ( int code, const std::string& documentFile, const std::string& error, int documentsParsed, int documentsSeen ) {
    std::stringstream event;

    switch(code) {
      case indri::api::IndexStatus::FileOpen:
        //event << "Opened " << documentFile;
        //buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileClose:
	// buildindex_print_status( "Documents parsed: ", documentsSeen, " Documents indexed: ", documentsParsed );
        //buildindex_print_event( "" );
        //event << "Closed " << documentFile;
        //buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileSkip:
        //event << "Skipped " << documentFile;
        //buildindex_print_event( event.str() ); 
        break;

      case indri::api::IndexStatus::FileError:
        //event << "Error in " << documentFile << " : " << error;
        //buildindex_print_event( event.str() ); 
        break;

      default:
      case indri::api::IndexStatus::DocumentCount:
        if( !(documentsSeen % 500) ) {
          //buildindex_print_status( "Documents parsed: ", documentsSeen, " Documents indexed: ", documentsParsed );
          //buildindex_flush_status();
        }
        break;
    }
  }
};

static std::string downcase_string( const std::string& str ) {
  std::string result;
  result.resize( str.size() );

  for( size_t i=0; i<str.size(); i++ ) {
    result[i] = tolower(str[i]);
  }
 
  return result;
}

static void downcase_string_vector (std::vector<std::string>& vec) {
  for( size_t i=0; i<vec.size(); i++ ) {
    vec[i] = downcase_string( vec[i] );
  }
}

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName, const std::string* subName = 0 ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];
  
  for( size_t i=0; i<slice.size(); i++ ) {
    if( subName ) {
      if( slice[i].exists(*subName) ) {
        vec.push_back( slice[i][*subName] );
      }
    } else {
      vec.push_back( slice[i] );
    }
  }

  return true;
}

/*! Given a Specification and a field name, return a vector containing all
 * of the field names that conflate to that name as well as the original
 * name.
 * @param spec The indri::parse::FileClassEnvironmentFactory::Specification to inspect.
 * @param name The field name to look for.
 * @return a vector containing all of the field names that conflate to that name as well as the original name.
 */
static std::vector<std::string> findConflations(indri::parse::FileClassEnvironmentFactory::Specification *spec, std::string & name) {
  std::vector<std::string> retval;
  // have to walk the map and add an entry for each
  // conflation to a given name
  std::map<indri::parse::ConflationPattern*, std::string>::const_iterator iter;
  for (iter = spec->conflations.begin(); 
       iter != spec->conflations.end(); iter++) {
    if( iter->second == name )
      retval.push_back(iter->first->tag_name);
  }
  // put the original into the list
  retval.push_back(name);
  return retval;
}

/*! Add a string to a vector if not already present.
 * @return true if the string was added.
 */
static bool addNew(std::vector<std::string>& orig, 
                   std::vector<std::string>& vec, string &name, 
                   std::string &specName, const char *msg) {
  bool retval = false;
  if( std::find( orig.begin(), orig.end(), name ) == orig.end() ) {
    std::cerr << "Adding " << name << " to " << specName << msg << std::endl;
    vec.push_back(name);
    retval = true;
  }
  return retval;
}
/*! Add field names to index or metadata for an existing file class 
 * specification.
 */
static bool augmentSpec(indri::parse::FileClassEnvironmentFactory::Specification *spec,
                        std::vector<std::string>& fields,
                        std::vector<std::string>& metadata,
                        std::vector<std::string>& metadataForward,
                        std::vector<std::string>& metadataBackward ) {
  // add to index and metadata fields in spec if necessary. 
  // return true if a field is changed.
  bool retval = false;
  // input field names are potentially conflated names:
  // eg headline for head, hl, or headline tags.
  std::vector<std::string> conflations;
  std::vector<std::string>::iterator i1;
  std::vector<std::string> origIndex = spec->index;
  std::vector<std::string> origInclude = spec->include;
  
  for (i1 = fields.begin(); i1 != fields.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) {
      // only add the field for indexing if it doesn't already exist
      if (addNew(origIndex, spec->index, conflations[j], 
                 spec->name, " as an indexed field")) {
        // added a field, make sure it is indexable
        // only add include tags if there are some already.
        // if it is empty, *all* tags are included.
        if( !spec->include.empty() ) {
          addNew(origInclude, spec->include, conflations[j], spec->name,
                 " as an included tag");
        }
        retval = true;
      }
    }
  }

  // add fields that should be marked metadata for retrieval
  for (i1 = metadata.begin(); i1 != metadata.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++)
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a metadata field");
  }
  // add fields that should have a metadata forward lookup table.
  for (i1 = metadataForward.begin(); i1 != metadataForward.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) 
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a forward indexed metadata field");
  }
  // add fields that should have a metadata reverse lookup table.
  for (i1 = metadataBackward.begin(); i1 != metadataBackward.end(); i1++) {
    // find any conflated names
    conflations = findConflations(spec, *i1);
    for (size_t j = 0; j < conflations.size(); j++) 
      retval |= addNew(spec->metadata, spec->metadata, conflations[j], spec->name,
                       " as a backward indexed metadata field");
  }

  return retval;
}

//
// process_numeric_fields
//

static void process_numeric_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) {
  std::string numName = "numeric";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  for( size_t i=0; i<slice.size(); i++ ) {
    bool isNumeric = slice[i].get(numName, false);
    if( isNumeric ) {
      // let user override default NumericFieldAnnotator for parser
      // enabling numeric fields in offset annotations.
      std::string parserName = slice[i].get("parserName", "NumericFieldAnnotator");
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setNumericField(fieldName, isNumeric, parserName);
    }
  }
}

//
// process_ordinal_fields
//

static void process_ordinal_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) {
  std::string ordName = "ordinal";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  
  for( size_t i=0; i<slice.size(); i++ ) {
    bool isOrdinal = slice[i].get(ordName, false);

    if( isOrdinal ) {
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setOrdinalField(fieldName, isOrdinal);
    }
  }
}

//
// process_parental_fields
//
static void process_parental_fields( indri::api::Parameters parameters, indri::api::IndexEnvironment& env ) { // pto
  std::string parName = "parental";
  std::string subName = "name";
  indri::api::Parameters slice = parameters["field"];
  for( int i=0; i<slice.size(); i++ ) {
    bool isParental = slice[i].get(parName, false);
    if( isParental ) {
      std::string fieldName = slice[i][subName];
      fieldName = downcase_string( fieldName );
      env.setParentalField(fieldName, isParental);
    }
  }
}

void require_parameter( const char* name, indri::api::Parameters& p ) {
  if( !p.exists( name ) ) {
    LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a " + name + " parameter." );
  }
}

std::string getAnchor(streamcorpus::StreamItem& streamItem) {
  std::string anchor;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("anchor");
    anchor = content.raw;
    return anchor;
  }
  catch(const std::out_of_range& orexpt) {
    return anchor;
  }
}

std::string getTitle(streamcorpus::StreamItem& streamItem) {
  std::string title;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("title");
    title  = content.raw;
    return title;
  }
  catch(const std::out_of_range& orexpt) {
    return title;
  }
}

std::vector<std::string> getKbaContent(std::string fileToParse ,const std::string directory) {
  using namespace indri::utility;
  using namespace indri::parse;
  std::vector<std::string> pdocs;
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(fileToParse);
  streamcorpus::StreamItem* streamItem = 0;
  std::string fileName = fileToParse.substr(fileToParse.rfind("/")+1);

  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    if((streamItem->body).clean_visible.size() <= 0) 
      continue;
    std::string streamId = streamItem->stream_id;
    std::string title = getTitle(*streamItem);
    std::string anchor = getAnchor(*streamItem);
    std::string content = title + " " + anchor + " " +(streamItem->body).clean_visible;
    std::string time = (streamItem->stream_id).substr(0, streamId.find("-"));
    std::string doc = "<doc>\n";
    doc = doc + "<docno>\n" + streamId + "\n</docno>" + "\n" + "<dir>\n"+directory+"\n</dir>\n"+ "<file>\n" + fileName + "\n</file>\n"+ "<time>\n"+time+ "\n</time>\n";
    doc = doc + "<text>\n" + content + "\n</text>" + "\n<title>\n" + title + "\n</title>" + "\n<anchor>\n" + anchor + "\n</anchor>\n" + "\n</doc>";
    pdocs.push_back(doc);
    //   std::cout << doc ;
  }
  
  delete tdextractor;
  return pdocs;
}
std::vector<indri::api::ParsedDocument> getKbaDocs(std::string fileToParse ,const std::string directory) {
  using namespace indri::utility;
  using namespace indri::parse;
  std::vector<indri::api::ParsedDocument> pdocs;
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(fileToParse);
  streamcorpus::StreamItem* streamItem = 0;
  std::string fileName = fileToParse.substr(fileToParse.rfind("/")+1);

  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    if((streamItem->body).clean_visible.size() <= 0) 
      continue;
    std::string streamId = streamItem->stream_id;
    indri::api::ParsedDocument pd;
    pd.textLength = 4;
    pd.text = strdup("rad");

    MetadataPair md;
    md.key = "docno";
    md.value = (streamItem->stream_id).c_str();
    md.valueLength = (streamItem->stream_id).size();
    pd.metadata.push_back(md);

    std::string time = (streamItem->stream_id).substr(0, streamId.find("-"));
    MetadataPair tm;
    tm.key = "time";
    tm.value = time.c_str();
    tm.valueLength = time.size();
    pd.metadata.push_back(tm);

    MetadataPair dirmd;
    dirmd.key = "dir";
    dirmd.value = directory.c_str();
    dirmd.valueLength = directory.size();
    pd.metadata.push_back(dirmd);
     
    MetadataPair filemd;
    filemd.key = "file";
    filemd.value = fileName.c_str();
    filemd.valueLength  = fileName.size();
    pd.metadata.push_back(filemd);
    pd.terms.push_back(strdup("rad"));
    /*
    std::vector<Sentence> sentences;
    sentences = (streamItem->body).sentences["lingpipe"]; 
    for(std::vector<Sentence>::iterator sentit = sentences.begin(); sentit != sentences.end(); sentit++) {
      Sentence  sentence = *sentit;
      for (std::vector<Token>::iterator tokIt = sentence.tokens.begin(); tokIt != sentence.tokens.end(); ++tokIt) {
        char* tok = (char*)((*tokIt).token.c_str());
        pd.terms.push_back(strdup(tok));
      }
    }
    */
    pdocs.push_back(pd);

  }

  delete tdextractor;
  return pdocs;
}

int createIndex(indri::api::Parameters& parameters, std::vector<std::string> dirList, const std::string repositoryPath, const std::string fileClass) {
  try {
    StatusMonitor monitor;
    indri::api::IndexEnvironment env;

    buildindex_start_time();

    if( parameters.get( "version", 0 ) ) {
      std::cout << INDRI_DISTRIBUTION << std::endl;
    }

    env.setMemory( parameters.get("memory", INT64(1024*1024*1024)) );

    env.setNormalization( parameters.get("normalize", true));
    env.setInjectURL( parameters.get("injectURL", false));
    env.setStoreDocs( parameters.get("storeDocs", false));
    /**
    std::string offsetAnnotationHint=parameters.get("offsetannotationhint", "default");
    if (offsetAnnotationHint=="ordered") {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintOrderedAnnotations);
    } if (offsetAnnotationHint=="unordered") {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintSizeBuffers);
    } else {
      env.setOffsetAnnotationIndexHint(indri::parse::OAHintDefault);
    }
    */
    std::string stemmerName = parameters.get("stemmer.name", "");
    if( stemmerName.length() )
      env.setStemmer(stemmerName);

    std::vector<std::string> stopwords;
    if( copy_parameters_to_string_vector( stopwords, parameters, "stopper.word" ) )
      env.setStopwords(stopwords);
    // fields to include as metadata (unindexed)
    
    std::vector<std::string> metadata;
    // metadata fields that should have a forward lookup table.
    std::vector<std::string> metadataForward;
    // metadata fields that should have a backward lookup table.
    std::vector<std::string> metadataBackward;
    copy_parameters_to_string_vector( metadata, parameters, "metadata.field" ); 
    downcase_string_vector(metadata);
    
    copy_parameters_to_string_vector( metadataForward, parameters, "metadata.forward" ); 
    downcase_string_vector(metadataForward);
    copy_parameters_to_string_vector( metadataBackward, parameters, "metadata.backward" );
    downcase_string_vector(metadataBackward);
    // docno is a special field, automagically add it as forward and backward.
    const std::string docno = "docno";
    if( std::find( metadataForward.begin(), 
                   metadataForward.end(), 
                   docno ) == metadataForward.end() )
      metadataForward.push_back(docno);
    if( std::find( metadataBackward.begin(), 
                   metadataBackward.end(), 
                   docno ) == metadataBackward.end() )
      metadataBackward.push_back(docno);

    env.setMetadataIndexedFields( metadataForward, metadataBackward );
    
    
    std::vector<std::string> fields;    
    std::string subName = "name";
    if( copy_parameters_to_string_vector( fields, parameters, "field", &subName ) ) {
      downcase_string_vector(fields);
      env.setIndexedFields(fields);
      process_numeric_fields( parameters, env );
      process_ordinal_fields( parameters, env );
      process_parental_fields( parameters, env ); //pto
    }
    
    if( indri::collection::Repository::exists( repositoryPath ) ) {
      // check if the repository was corrupted by an indexing crash
      // if so, recover it and continue.
      if (_recoverRepository(repositoryPath)) {
        env.open( repositoryPath, &monitor );
        buildindex_print_event( std::string() + "Opened repository " + repositoryPath ); 
      } else  {
        //  failed to open it, needs to be created from scratch.
        // create will remove any cruft.
        env.create( repositoryPath, &monitor );
        buildindex_print_event( std::string() + "Created repository " + repositoryPath );
      }
    } else {
      env.create( repositoryPath, &monitor );
      buildindex_print_event( std::string() + "Created repository " + repositoryPath );
    }

    for (std::vector<std::string>::iterator dirIt =  dirList.begin(); dirIt != dirList.end(); ++dirIt) {
      std::string corpusPath = *dirIt;
      std::string directory = corpusPath.substr(corpusPath.rfind("/")+1);

      // augment field/metadata tags in the environment if needed.
      
      if( fileClass.length() ) {
        indri::parse::FileClassEnvironmentFactory::Specification *spec = env.getFileClassSpec(fileClass);
        if( spec ) {
          // add fields if necessary, only update if changed.
          if( augmentSpec( spec, fields, metadata, metadataForward, metadataBackward ) ) 
            env.addFileClass(*spec);
          delete(spec);
        }
      }
      
      bool isDirectory = indri::file::Path::isDirectory(corpusPath);
 
      // First record the document root, and then the paths to any annotator inputs
      env.setDocumentRoot(corpusPath);

      // Support for anchor text
      //std::string anchorText = thisCorpus.get("inlink", "");
      //env.setAnchorTextPath("");

      //      Support for offset annotations
      //std::string offsetAnnotationsPath = thisCorpus.get( "annotations", "" );
      //env.setOffsetAnnotationsPath("");

      // Support for offset metadata file
      //std::string offsetMetadataPath = thisCorpus.get( "metadata", "" );
      //env.setOffsetMetadataPath("");

      if( isDirectory ) {
        indri::file::FileTreeIterator files(corpusPath);
          for( ; files != indri::file::FileTreeIterator::end(); files++ ) {
	  std::string file = *files;
	  //	  std::vector<indri::api::ParsedDocument> pdocs = getKbaDocs(file, directory);
	  std::vector<std::string> pdocs = getKbaContent(file, directory);
          for(std::vector<std::string>::iterator pdIt = pdocs.begin(); pdIt != pdocs.end(); ++pdIt) {
	    std::string pd = *pdIt;
	    //	    std::cout << "doc size " << pd.size() << "\n";
            //env.addParsedDocument(&pd);
	    std::vector<indri::parse::MetadataPair> md;
            env.addString(pd, fileClass, md);
	  }
        }
      } 
    }

    buildindex_print_event( "Closing index" );
    env.close();
    buildindex_print_event( "Finished" );
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }

  return 0;
}

bool compareString(std::string firstStr, std::string secondStr) {
  if(firstStr.compare(secondStr) < 0) 
    return true;
  return false;
}


int main(int argc, char* argv[]) {
  using namespace boost::program_options;
  std::vector<std::string> paramFiles;
  std::string dirListFile;
  std::string baseRepoPath;
  
 
  const std::string fileClass = "trectext";
  std::vector<std::string> dirList;

  std::string baseCorpusPath; 

  options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("param", value<std::vector<std::string> >(&paramFiles))
    ("repo", value<std::string >(&baseRepoPath)->default_value("/data/data/collections/KBA/2013/index"))
    ("corpus", value<std::string >(&baseCorpusPath)->default_value("../help/corpus"))
    ("dir-list", value<std::string >(&dirListFile));

  variables_map cmndMap;
  store(parse_command_line(argc, argv,cmndDesc), cmndMap);
  notify(cmndMap);  
  
  indri::api::Parameters& parameters = indri::api::Parameters::instance();
  for(std::vector<std::string>::iterator paramIt = paramFiles.begin(); paramIt != paramFiles.end(); ++paramIt) {
    parameters.loadFile(*paramIt);
  }
     
  std::ifstream inputFile(cmndMap["dir-list"].as<std::string>().c_str());
  for(std::string line;getline(inputFile, line);) {
    dirList.push_back(line);
  }
  inputFile.close();
  std::sort(dirList.begin(), dirList.end(), compareString);
  std::vector<std::string> dirBunch;
  std::string prevDayDate = (dirList.at(0)).substr(0, (dirList.at(0)).rfind('-'));

  for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
    std::string dir = *dirIt;
    std::string dayDate = dir.substr(0, dir.rfind('-'));
  
    if(dayDate.compare(prevDayDate) != 0) {
      std::string repoPath = baseRepoPath + "/" + dayDate;
      //      std::cout << dirBunch.size() << "\n";
      int status = createIndex(parameters, dirBunch, repoPath, fileClass);
      if(status < 0)
        std::cout << "Could not create index for dir" << dayDate << "\n";
      dirBunch.clear();
    }

    std::string corpusPath = baseCorpusPath + "/" + dir;
    dirBunch.push_back(corpusPath);
    prevDayDate  = dayDate;

  }

  if(dirBunch.size() > 0) {
    std::string repoPath = baseRepoPath + "/" + prevDayDate;
    int status = createIndex(parameters, dirBunch, repoPath, fileClass);
    if(status < 0)
      std::cout << "Could not create index for dir" << prevDayDate << "\n";
  }

  return 0;
}
