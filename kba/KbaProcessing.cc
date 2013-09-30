#include "ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/tokenizer.hpp"
#include <boost/shared_ptr.hpp>
#include "Tokenize.hpp"
#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include <fstream>
#include <vector>
#include <iostream>
#include "BaseLineScorer.hpp"
#include "DumpKbaResult.hpp"
#include "StreamThread.hpp"
#include <boost/thread.hpp>

namespace cmndOp = boost::program_options;

std::vector<kba::entity::Entity*> ENTITY_SET;

void runScoring(std::string entityfile, std::string ) {
}

void iterateOnStream(std::string& fileName, cmndOp::variables_map& cmndMap, std::string& taggerId) {
  kba::thrift::ThriftDocumentExtractor docExtractor;   

  docExtractor.open(fileName);
  StreamItem *streamItem;
  while((streamItem = docExtractor.nextStreamItem()) != NULL) {
    if(cmndMap.count("anchor"))
        docExtractor.getAnchor(*streamItem);
    if(cmndMap.count("title"))
      docExtractor.getTitle(*streamItem); 
    if(cmndMap.count("sentence")) {
      docExtractor.iterateOverSentence(*streamItem, taggerId); 
    }
  }
}



void performCCRTask(std::string entityfile, std::string pathToProcess, std::string fileToDump) {
   
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  std::cout << "total enti : " << ENTITY_SET.size() << (ENTITY_SET[0])->wikiURL << "\n"; 
  bool isDirectory = indri::file::Path::isDirectory(pathToProcess );   
 
  indri::file::FileTreeIterator files(pathToProcess);
  kba::dump::writeHeader(fileToDump);
  std::fstream* dumpStream = new std::fstream(fileToDump.c_str(), std::fstream::out | std::fstream::app);

  if(!isDirectory) {
        kba::scorer::BaseLineScorer bscorer(ENTITY_SET); 
        kba::StreamThread st(pathToProcess, dumpStream, &bscorer, 0);
        st.parseFile();
  }
  else {
    int index = 0;
    boost::thread_group threadGroup;
    boost::mutex* lockMutex = new boost::mutex;
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      if( index > 3) {
        std::cout << "waiting for threads to finish\n";
        threadGroup.join_all();
        std::cout << "Fiinish\n";
        index = 0;
      }
        
      std::string fileName(*files);
      kba::scorer::BaseLineScorer bscorer(ENTITY_SET); 
      std::cout << "process : " << fileName << "\n";   
      kba::StreamThread st(fileName, dumpStream, &bscorer, lockMutex);
      std::cout << "creating thread \n";
      //     st.parseFile();
         threadGroup.create_thread(st);
      //st.parseFile();
      index++;

    }
    
    if(index > 0) {
     threadGroup.join_all();
     } 
     
    delete lockMutex;
  }
  
  dumpStream->close(); 
  delete dumpStream;
}

int main(int argc, char *argv[]){
  std::string taggerId;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help","the help message")
    ("file",cmndOp::value<std::string>(),"the file or base dir to process")
    ("efile",cmndOp::value<std::string>(),"The full path of the entity json file")
    ("dfile",cmndOp::value<std::string>(),"The dump file")
    ("anchor"," print anchor text")
    ("title"," print title text")
    ("sentence"," print sentences ")
    ("repo",cmndOp::value<std::string>(), "Repository path of the rdfs store")
    ("repo-name", cmndOp::value<std::string>(), "The name of repository in the repository path") 
    ("print-model", "print the model")
    ("equery",cmndOp::value<std::string>(), "Look up an entity")
    ("taggerId",cmndOp::value<std::string>(&taggerId)->default_value("lingpipe")," print anchor text");

  cmndOp::variables_map cmndMap;
  cmndOp::store(cmndOp::parse_command_line(argc, argv,cmndDesc), cmndMap);
  cmndOp::notify(cmndMap);  
 
 
  RDFParser* rdfparser = NULL; 
  if(cmndMap.count("repo") && cmndMap.count("repo-name")) {
    rdfparser = new RDFParser();   
    std::string repoName = cmndMap["repo-name"].as<std::string>();
    std::string repo = cmndMap["repo"].as<std::string>();
    rdfparser->initRDFParser(repoName, repo);

    if(cmndMap.count("print-model")) {
      rdfparser->streamModel(stdout);   
    }

    RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld());
    if(cmndMap.count("equery")) {
      std::string equery = cmndMap["equery"].as<std::string>();
 
      std::cout << "Executing : " << equery <<"\n";
      std::vector< boost::shared_ptr<unsigned char> >  nodes = rdfquery->getSourceNodes((const unsigned char*)"http://xmlns.com/foaf/0.1/homepage", (const unsigned char*)equery.c_str(), false);
      for(std::vector<boost::shared_ptr<unsigned char> >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); nodeIt++) {
	boost::shared_ptr<unsigned char> nodeValue = *nodeIt;
	std::cout << "\n found nodes are "<< nodeValue.get(); 
      }
    }
  }
  
  if(cmndMap.count("efile") && cmndMap.count("dfile") && cmndMap.count("file")) {
    std::string entityFile = cmndMap["efile"].as<std::string>();
    std::string dumpFile = cmndMap["dfile"].as<std::string>();
    std::string pathToProcess = cmndMap["file"].as<std::string>();
    performCCRTask(entityFile, pathToProcess, dumpFile);
  }

  if (!cmndMap.count("file")) {
    std::cout <<  "no input file specified use --file \n";
    return -1;
  }
 
  std::string basePath = cmndMap["file"].as<std::string>();
  bool isDirectory = indri::file::Path::isDirectory(basePath );   
  indri::file::FileTreeIterator files(basePath);
  if(!isDirectory)
    iterateOnStream(basePath, cmndMap, taggerId);
  else {
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      
      std::string fileName(*files);
      iterateOnStream(fileName, cmndMap, taggerId);
    }
  }
  return 0;
}
