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

//
// runquery
//
// 24 February 2004 -- tds
//
// 18 August 2004 -- dam
// incorporated multiple query, query expansion, and TREC output support
//
//
// Indri local machine query application
///*! \page IndriRunQuery Indri Query Retrieval

#include <time.h>
#include "indri/QueryEnvironment.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/delete_range.hpp"
#include "indri/NetworkStream.hpp"
#include "indri/NetworkMessageStream.hpp"
#include "indri/NetworkServerProxy.hpp"

#include "indri/ListIteratorNode.hpp"
#include "indri/ExtentInsideNode.hpp"
#include "indri/DocListIteratorNode.hpp"
#include "indri/FieldIteratorNode.hpp"

#include "indri/Parameters.hpp"

#include "indri/ParsedDocument.hpp"
#include "indri/Collection.hpp"
#include "indri/CompressedCollection.hpp"
#include "indri/TaggedDocumentIterator.hpp"
#include "indri/XMLNode.hpp"

#include "indri/QueryExpander.hpp"
#include "indri/RMExpander.hpp"
#include "indri/PonteExpander.hpp"
// need a QueryExpanderFactory....
#include "indri/TFIDFExpander.hpp"

#include "indri/IndriTimer.hpp"
#include "indri/UtilityThread.hpp"
#include "indri/ScopedLock.hpp"
#include "indri/delete_range.hpp"
#include "indri/SnippetBuilder.hpp"

#include <queue>

#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"

#ifdef _cplusplus
extern "C" {
#endif
#include <libxml/tree.h>
#include <libxml/parser.h>
#ifdef _cplusplus
}
#endif

#include "Query.hpp"
#include "PassageModel.hpp"
std::map<std::string, Query*> constructQuery(std::string queryFile) {
  LIBXML_TEST_VERSION
  xmlDoc* doc = 0;
  std::map<std::string, Query*>();
  doc = xmlReadFile(queryFile.c_str(), NULL, 0); // DOM tree
  std::map<std::string, Query*> queryMap;
  if(doc == 0) {
    std::cout << "Could not parse xml topic file ";
    return queryMap;
  }
  xmlNode* node = (xmlDocGetRootElement(doc))->children; // Sibling of the root 
  
  for( ;node; node = node->next) {
    std::string nodeName = (const char*)(node->name);
    //    std::cout << nodeName << "\n" ; 
    //<< (const char*)(node->_private) <<"\n";
    if(node->type == XML_ELEMENT_NODE && nodeName.compare("topic") == 0) {
      Query* q = new Query();
      xmlAttr* attr  = node->properties;      
      std::string attName = (const char*)(attr->name);
      //      std::cout << attName << "\n";
      std::string data = (const char*)((attr->children)->content);
      //std::cout << data << "\n";
      if(attName.compare("number") == 0) {
        q->qnum = data;
        queryMap.insert(std::pair<std::string, Query*>(data, q));
      }
      else
        q->queryType = data;

      attr = attr->next;
      attName = (const char*)(attr->name); 
      data = (const char*)((attr->children)->content);
      //     std::cout << attName << " : "<< data << "\n"; 
      if(attName.compare("type") == 0)
        q->queryType = data;
      else {
        q->qnum = data;
        queryMap.insert(std::pair<std::string, Query*>(data, q));
      }

      xmlNode* child = node->children;
      for(;child; child = child->next) {
        if(child->type != XML_ELEMENT_NODE)
          continue;
	std::string subName = (const char*)(child->name);
	//	std::cout << subName << "\n"; 	
        if(subName.compare("query") == 0) {
          std::string data = (const char*) ((child->children)->content);
          q->query = data;
	}
        else if(subName.compare("description") == 0) {
	  std::string data = (const char*) ((child->children)->content);
          q->description = data;
	}
        else if(subName.compare("subtopic") == 0) {
	  std::string data = (const char*) ((child->children)->content);
          Query* subq = new Query();
          subq->description = data;
          xmlAttr* subattr = child->properties;
	  std::string sAttrName = (const char*)(subattr->name);
          data = (const char*) ((subattr->children)->content);
          if(sAttrName.compare("number") == 0) {
            subq->qnum = data;
	  }
          else
            subq->queryType = data;
          
          subattr = subattr->next;
          sAttrName = (const char*)(subattr->name);
          data = (const char*)((subattr->children)->content);
 
          if(sAttrName.compare("number") == 0) {
            subq->qnum = data;
	  }
          else
            subq->queryType = data;
          (q->subquery).push_back(subq);
	  //  std::cout << " ==subquery== " << subq->description << " : " << subq->qnum << " : " << subq->queryType << "\n";
	}
      }
      //      std::cout << "Adding query :" << q->query << " : " << q->qnum << " : " << q->description << " : " << q->queryType  << "\n";
      //      querySet.push_back(q);
    }
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();
  return queryMap;
}

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];

  for( size_t i=0; i<slice.size(); i++ ) {
    vec.push_back( slice[i] );
  }

  return true;
}

struct query_t {
  struct greater {
    bool operator() ( query_t* one, query_t* two ) {
      return one->index > two->index;
    }
  };

  query_t( int _index, std::string _number, const std::string& _text, const std::string &queryType,  std::vector<std::string> workSet,   std::vector<std::string> FBDocs) :
    index( _index ),
    number( _number ),
    text( _text ), qType(queryType), workingSet(workSet), relFBDocs(FBDocs)
  {
  }

  query_t( int _index, std::string _number, const std::string& _text ) :
    index( _index ),
    number( _number ),
    text( _text )
  {
  }

  std::string number;
  int index;
  std::string text;
  std::string qType;
  // working set to restrict retrieval
  std::vector<std::string> workingSet;
  // Rel fb docs
  std::vector<std::string> relFBDocs;
};

class QueryThread : public indri::thread::UtilityThread {
private:
  indri::thread::Lockable& _queueLock;
  indri::thread::ConditionVariable& _queueEvent;
  std::queue< query_t* >& _queries;
  std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater >& _output;

  indri::api::QueryEnvironment _environment;
  indri::api::Parameters& _parameters;
  int _requested;
  int _initialRequested;

  bool _printDocuments;
  bool _printPassages;
  bool _printSnippets;
  bool _printQuery;

  std::string _runID;
  bool _trecFormat;
  bool _inexFormat;

  indri::query::QueryExpander* _expander;
  std::vector<indri::api::ScoredExtentResult> _results;
  indri::api::QueryAnnotation* _annotation;

  std::unordered_set<std::string> _stopwords;
  std::map<std::string, Query*> _queryMap;

  // Runs the query, expanding it if necessary.  Will print output as well if verbose is on.
  void _runQuery( std::stringstream& output, const std::string& query, const std::string& qnumber,
                  const std::string &queryType, const std::vector<std::string> &workingSet, std::vector<std::string> relFBDocs ) {
    try {
      if( _printQuery ) output << "# query: " << query << std::endl;
      std::vector<lemur::api::DOCID_T> docids;;
      if (workingSet.size() > 0) 
        docids = _environment.documentIDsFromMetadata("docno", workingSet);

      if (relFBDocs.size() == 0) {
          if( _printSnippets ) {
            if (workingSet.size() > 0) 
              _annotation = _environment.runAnnotatedQuery( query, docids, _initialRequested, queryType ); 
            else
              _annotation = _environment.runAnnotatedQuery( query, _initialRequested );
            _results = _annotation->getResults();
          } else {
            if (workingSet.size() > 0)
              _results = _environment.runQuery( query, docids, _initialRequested, queryType );
            else
              _results = _environment.runQuery( query, _initialRequested, queryType );
          }
      }
      
      if( _expander ) {
        std::vector<indri::api::ScoredExtentResult> fbDocs;
        if (relFBDocs.size() > 0) {
          docids = _environment.documentIDsFromMetadata("docno", relFBDocs);
          for (size_t i = 0; i < docids.size(); i++) {
            indri::api::ScoredExtentResult r(0.0, docids[i]);
            fbDocs.push_back(r);
          }
        }
        std::string expandedQuery;
        if (relFBDocs.size() != 0)
          expandedQuery = _expander->expand( query, fbDocs );
        else
          expandedQuery = _expander->expand( query, _results );
        if( _printQuery ) output << "# expanded: " << expandedQuery << std::endl;
        if (workingSet.size() > 0) {
          docids = _environment.documentIDsFromMetadata("docno", workingSet);
          _results = _environment.runQuery( expandedQuery, docids, _requested, queryType );
        } else {
          _results = _environment.runQuery( expandedQuery, _requested, queryType );
        }
      }

      Query* q = _queryMap[qnumber];
      _results  = PassageModel::maxPsgScoring(&_environment, q, _results, true, _stopwords, 150, 50);
       
    }
    catch( lemur::api::Exception& e )
    {
      _results.clear();
      LEMUR_RETHROW(e, "QueryThread::_runQuery Exception");
    }
  }

  void _printResultRegion( std::stringstream& output, std::string queryIndex, int start, int end  ) {
    std::vector<std::string> documentNames;
    std::vector<indri::api::ParsedDocument*> documents;

    std::vector<indri::api::ScoredExtentResult> resultSubset;

    resultSubset.assign( _results.begin() + start, _results.begin() + end );


    // Fetch document data for printing
    if( _printDocuments || _printPassages || _printSnippets ) {
      // Need document text, so we'll fetch the whole document
      documents = _environment.documents( resultSubset );
      documentNames.clear();

      for( size_t i=0; i<resultSubset.size(); i++ ) {
        indri::api::ParsedDocument* doc = documents[i];
        std::string documentName;

        indri::utility::greedy_vector<indri::parse::MetadataPair>::iterator iter = std::find_if( documents[i]->metadata.begin(),
          documents[i]->metadata.end(),
          indri::parse::MetadataPair::key_equal( "docno" ) );

        if( iter != documents[i]->metadata.end() )
          documentName = (char*) iter->value;

        // store the document name in a separate vector so later code can find it
        documentNames.push_back( documentName );
      }
    } else {
      // We only want document names, so the documentMetadata call may be faster
      documentNames = _environment.documentMetadata( resultSubset, "docno" );
    }

    std::vector<std::string> pathNames;
    if ( _inexFormat ) {
      // retrieve path names
      pathNames = _environment.pathNames( resultSubset );
    }

    // Print results
    for( size_t i=0; i < resultSubset.size(); i++ ) {
      int rank = start+i+1;
      std::string queryNumber = queryIndex;

      if( _trecFormat ) {
        // TREC formatted output: queryNumber, Q0, documentName, rank, score, runID
        output << queryNumber << " "
                << "Q0 "
                << documentNames[i] << " "
                << rank << " "
                << resultSubset[ i ].score << " "
                << _runID << std::endl;
      } else if( _inexFormat ) {

  output << "    <result>" << std::endl
         << "      <file>" << documentNames[i] << "</file>" << std::endl
         << "      <path>" << pathNames[i] << "</path>" << std::endl
         << "      <rsv>" << resultSubset[i].score << "</rsv>"  << std::endl
         << "    </result>" << std::endl;
      }
      else {
        // score, documentName, firstWord, lastWord
        output << resultSubset[i].score << "\t"
                << documentNames[i] << "\t"
                << resultSubset[i].begin << "\t"
                << resultSubset[i].end << std::endl;
      }

      if( _printDocuments ) {
        output << documents[i]->text << std::endl;
      }

      if( _printPassages ) {
        int byteBegin = documents[i]->positions[ resultSubset[i].begin ].begin;
        int byteEnd = documents[i]->positions[ resultSubset[i].end-1 ].end;
        output.write( documents[i]->text + byteBegin, byteEnd - byteBegin );
        output << std::endl;
      }

      if( _printSnippets ) {
        indri::api::SnippetBuilder builder(false);
        output << builder.build( resultSubset[i].document, documents[i], _annotation ) << std::endl;
      }

      if( documents.size() )
        delete documents[i];
    }
  }

  void _printResults( std::stringstream& output, std::string queryNumber ) {
    if (_inexFormat) {
      // output topic header
      output << "  <topic topic-id=\"" << queryNumber << "\">" << std::endl
             << "    <collections>" << std::endl
             << "      <collection>ieee</collection>" << std::endl
             << "    </collections>" << std::endl;
    }
    for( size_t start = 0; start < _results.size(); start += 50 ) {
      size_t end = std::min<size_t>( start + 50, _results.size() );
      _printResultRegion( output, queryNumber, start, end );
    }
    if( _inexFormat ) {
      output << "  </topic>" << std::endl;
    }
    delete _annotation;
    _annotation = 0;
  }


public:
  QueryThread( std::queue< query_t* >& queries,
               std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater >& output,
               indri::thread::Lockable& queueLock,
               indri::thread::ConditionVariable& queueEvent,
               indri::api::Parameters& params, std::map<std::string, Query*> queryMap ) :
    _queries(queries),
    _output(output),
    _queueLock(queueLock),
    _queueEvent(queueEvent),
    _parameters(params),
    _expander(0),
    _annotation(0),
    _queryMap(queryMap)
  {
  }

  ~QueryThread() {
  }

  UINT64 initialize() {
    _environment.setSingleBackgroundModel( _parameters.get("singleBackgroundModel", false) );

    std::vector<std::string> stopwords;
    if( copy_parameters_to_string_vector( stopwords, _parameters, "stopper.word" ) )
      _environment.setStopwords(stopwords);

    for(std::vector<std::string>::iterator wordIt = stopwords.begin(); wordIt != stopwords.end(); ++wordIt)
      _stopwords.insert(*wordIt);

    std::vector<std::string> smoothingRules;
    if( copy_parameters_to_string_vector( smoothingRules, _parameters, "rule" ) )
      _environment.setScoringRules( smoothingRules );

   if( _parameters.exists( "index" ) ) {
      indri::api::Parameters indexes = _parameters["index"];

      for( size_t i=0; i < indexes.size(); i++ ) {
        _environment.addIndex( std::string(indexes[i]) );
      }
    }

    if( _parameters.exists( "server" ) ) {
      indri::api::Parameters servers = _parameters["server"];

      for( size_t i=0; i < servers.size(); i++ ) {
        _environment.addServer( std::string(servers[i]) );
      }
    }

    if( _parameters.exists("maxWildcardTerms") )
        _environment.setMaxWildcardTerms(_parameters.get("maxWildcardTerms", 100));

    _requested = _parameters.get( "count", 1000 );
    _initialRequested = _parameters.get( "fbDocs", _requested );
    _runID = _parameters.get( "runID", "indri" );
    _trecFormat = _parameters.get( "trecFormat" , false );
    _inexFormat = _parameters.exists( "inex" );

    _printQuery = _parameters.get( "printQuery", false );
    _printDocuments = _parameters.get( "printDocuments", false );
    _printPassages = _parameters.get( "printPassages", false );
    _printSnippets = _parameters.get( "printSnippets", false );

    if (_parameters.exists("baseline")) {
      // doing a baseline
      std::string baseline = _parameters["baseline"];
      _environment.setBaseline(baseline);
      // need a factory for this...
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        // have to push the method in...
        std::string rule = "method:" + baseline;
        _parameters.set("rule", rule);
        _expander = new indri::query::TFIDFExpander( &_environment, _parameters );
      }
    } else {
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        _expander = new indri::query::RMExpander( &_environment, _parameters );
      }
    }

    if (_parameters.exists("maxWildcardTerms")) {
      _environment.setMaxWildcardTerms((int)_parameters.get("maxWildcardTerms"));
    }    
    return 0;
  }

  void deinitialize() {
    delete _expander;
    _environment.close();
  }

  bool hasWork() {
    indri::thread::ScopedLock sl( &_queueLock );
    return _queries.size() > 0;
  }

  UINT64 work() {
    query_t* query;
    std::stringstream output;

    // pop a query off the queue
    {
      indri::thread::ScopedLock sl( &_queueLock );
      if( _queries.size() ) {
        query = _queries.front();
        _queries.pop();
      } else {
        return 0;
      }
    }

    // run the query
    try {
      if (_parameters.exists("baseline") && ((query->text.find("#") != std::string::npos) || (query->text.find(".") != std::string::npos)) ) {
        LEMUR_THROW( LEMUR_PARSE_ERROR, "Can't run baseline on this query: " + query->text + "\nindri query language operators are not allowed." );
      }
      //      std::cout << query->text << "\n";
      _runQuery( output, query->text, query->number, query->qType, query->workingSet, query->relFBDocs );
    } catch( lemur::api::Exception& e ) {
      output << "# EXCEPTION in query " << query->number << ": " << e.what() << std::endl;
    }

    // Do some post processing here
    // print the results to the output stream
    _printResults( output, query->number );

    // push that data into an output queue...?
    {
      indri::thread::ScopedLock sl( &_queueLock );
      _output.push( new query_t( query->index, query->number, output.str() ) );
      _queueEvent.notifyAll();
    }

    delete query;
    return 0;
  }
};

void push_queue( std::queue< query_t* >& q, indri::api::Parameters& queries,
                 int queryOffset ) {

  for( size_t i=0; i<queries.size(); i++ ) {
    std::string queryNumber;
    std::string queryText;
    std::string queryType = "indri";
    if( queries[i].exists( "type" ) )
      queryType = (std::string) queries[i]["type"];
    if (queries[i].exists("text"))
      queryText = (std::string) queries[i]["text"];
    if( queries[i].exists( "number" ) ) {
      queryNumber = (std::string) queries[i]["number"];
    } else {
      int thisQuery=queryOffset + int(i);
      std::stringstream s;
      s << thisQuery;
      queryNumber = s.str();
    }
    if (queryText.size() == 0)
      queryText = (std::string) queries[i];

    // working set and RELFB docs go here.
    // working set to restrict retrieval
    std::vector<std::string> workingSet;
    // Rel fb docs
    std::vector<std::string> relFBDocs;
    copy_parameters_to_string_vector( workingSet, queries[i], "workingSetDocno" );
    copy_parameters_to_string_vector( relFBDocs, queries[i], "feedbackDocno" );

    q.push( new query_t( i, queryNumber, queryText, queryType, workingSet, relFBDocs ) );

  }
}

int main(int argc, char * argv[]) {
  try {
    using namespace boost::program_options;
    std::string topicFile;
    std::vector<std::string> paramFiles;
    options_description cmndDesc("Allowed command line options");
    cmndDesc.add_options()
      ("qfile", value<std::string>(&topicFile)->default_value("/usa/arao/trec/trec-web/clueweb12/trec2013-topics.xml"))
      ("param", value<std::vector<std::string> >(&paramFiles));
    variables_map cmndMap;
    store(parse_command_line(argc, argv,cmndDesc), cmndMap);
    notify(cmndMap);  
   
    std::map<std::string, Query*> origQuery = constructQuery(topicFile);  
    indri::api::Parameters param = indri::api::Parameters::instance();
    for(std::vector<std::string>::iterator fIt = paramFiles.begin(); fIt != paramFiles.end(); ++fIt) {
      param.loadFile(*fIt);
    }

    if( param.get( "version", 0 ) ) {
      std::cout << INDRI_DISTRIBUTION << std::endl;
    }

    if( !param.exists( "query" ) )
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify at least one query." );

    if( !param.exists("index") && !param.exists("server") )
      LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a server or index to query against." );

    if (param.exists("baseline") && param.exists("rule"))
      LEMUR_THROW( LEMUR_BAD_PARAMETER_ERROR, "Smoothing rules may not be specified when running a baseline." );

    int threadCount = param.get( "threads", 1 );
    std::queue< query_t* > queries;
    std::priority_queue< query_t*, std::vector< query_t* >, query_t::greater > output;
    std::vector< QueryThread* > threads;
    indri::thread::Mutex queueLock;
    indri::thread::ConditionVariable queueEvent;

    // push all queries onto a queue
    indri::api::Parameters parameterQueries = param[ "query" ];
    int queryOffset = param.get( "queryOffset", 0 );
    push_queue( queries, parameterQueries, queryOffset );
    int queryCount = (int)queries.size();

    // launch threads
    for( int i=0; i<threadCount; i++ ) {
      threads.push_back( new QueryThread( queries, output, queueLock, queueEvent, param , origQuery) );
      threads.back()->start();
    }

    int query = 0;

    bool inexFormat = param.exists( "inex" );
    if( inexFormat ) {
      std::string participantID = param.get( "inex.participantID", "1");
      std::string runID = param.get( "runID", "indri" );
      std::string inexTask = param.get( "inex.task", "CO.Thorough" );
      std::string inexTopicPart = param.get( "inex.topicPart", "T" );
      std::string description = param.get( "inex.description", "" );
      std::string queryType = param.get("inex.query", "automatic");
      std::cout << "<inex-submission participant-id=\"" << participantID
    << "\" run-id=\"" << runID
    << "\" task=\"" << inexTask
    << "\" query=\"" << queryType
    << "\" topic-part=\"" << inexTopicPart
    << "\">" << std::endl
    << "  <description>" << std::endl << description
    << std::endl << "  </description>" << std::endl;
    }

    // acquire the lock.
    queueLock.lock();

    // process output as it appears on the queue
    while( query < queryCount ) {
      query_t* result = NULL;

      // wait for something to happen
      queueEvent.wait( queueLock );

      while( output.size() && output.top()->index == query ) {
        result = output.top();
        output.pop();

        queueLock.unlock();

        std::cout << result->text;
        delete result;
        query++;

        queueLock.lock();
      }
    }
    queueLock.unlock();

    if( inexFormat ) {
      std::cout << "</inex-submission>" << std::endl;
    }

    // join all the threads
    for( size_t i=0; i<threads.size(); i++ )
      threads[i]->join();

    // we've seen all the query output now, so we can quit
    indri::utility::delete_vector_contents( threads );
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  } catch( ... ) {
    std::cout << "Caught unhandled exception" << std::endl;
    return -1;
  }

  return 0;
}

