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
#include <iostream>
#include <vector>
#include <algorithm>
#include <iostream>
#include "BaseLineScorer.hpp"
#include "DumpKbaResult.hpp"
#include "StreamThread.hpp"
#include <boost/thread.hpp>
#include <time.h>
namespace cmndOp = boost::program_options;

std::vector<kba::entity::Entity*> ENTITY_SET;


bool compareString(std::string firstStr, std::string secondStr) {
  if(firstStr.compare(secondStr) < 0) 
    return true;
  return false;
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

void findStreamId(std::string& fileName, std::string streamId) {
  kba::thrift::ThriftDocumentExtractor docExtractor;   

  docExtractor.open(fileName);
  StreamItem *streamItem;
  while((streamItem = docExtractor.nextStreamItem()) != NULL) {
    if(!(streamItem->stream_id).compare(streamId)) {
      std::cout << "*********Stream Content***********\n";
      std::cout << (streamItem->body).clean_visible;
    }     
  }
}



void performCCRTask(std::string entityfile, std::string pathToProcess, std::string fileToDump, std::vector<std::string> dirList) {
  int cutOffScore=500; 
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  kba::entity::updateEntityWithDbpedia(ENTITY_SET, "/usa/arao/dbpediadumps/dbpedia7bdb", "wikiToDb");
  kba::entity::updateEntityWithLabels(ENTITY_SET, "/usa/arao/dbpediadumps/dbpedia7bdb", "labels");
  std::vector<kba::entity::Entity*> filterSet;
  for(std::vector<kba::entity::Entity*>::iterator entityIt = ENTITY_SET.begin(); entityIt != ENTITY_SET.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    if((entity->label).size() > 0 || entity->dbpediaURLs.size() > 0)
      filterSet.push_back(entity);
  } 
  
  ENTITY_SET = filterSet;
  std::cout << "total enti : " << ENTITY_SET.size() << (ENTITY_SET[0])->wikiURL << "\n"; 
  bool isDirectory = indri::file::Path::isDirectory(pathToProcess );   
 
  
  kba::dump::writeHeader(fileToDump);
  std::fstream* dumpStream = new std::fstream(fileToDump.c_str(), std::fstream::out | std::fstream::app);
  time_t startTime;
  time(&startTime);

  if(!isDirectory) {
        kba::scorer::BaseLineScorer bscorer(ENTITY_SET); 
	std::string fileExtension = pathToProcess.substr(pathToProcess.size() - 3);
        if(!fileExtension.compare(".xz")) {
          kba::StreamThread st(pathToProcess, dumpStream, &bscorer, 0);
          st.parseFile(cutOffScore);
	}
  }
  else {
    int index = 0;
    boost::thread_group threadGroup;
    boost::mutex* lockMutex = new boost::mutex;
    kba::scorer::BaseLineScorer bscorer(ENTITY_SET); 
    std::vector<std::string> pathList;
    std::vector<boost::thread*> aliveThreads;
    int maxThreads = 4;

    if(dirList.size() <= 0) 
      pathList.push_back(pathToProcess);
    else {
      for(int index=0; index < dirList.size(); index++) {
	std::string subdir = dirList[index];
        pathList.push_back(pathToProcess + "/"+ subdir); 
      }
    }

    for(std::vector<std::string>::iterator pathIt = pathList.begin(); pathIt != pathList.end(); pathIt++) {
      pathToProcess = *pathIt; 
      indri::file::FileTreeIterator files(pathToProcess);
      for(; files != indri::file::FileTreeIterator::end() ;files++) {
        if( index >= maxThreads) {
	  //    std::cout << "waiting for threads to finish\n";
          threadGroup.join_all();
	  //          for(int idx=0; idx < maxThreads ; idx++) {
	  // delete aliveThreads[idx];
	  // }
          aliveThreads.clear();
          index = 0;
        }
        
        std::string fileName(*files);
        std::string fileExtension = fileName.substr(fileName.size() - 3);
      
        if(!fileExtension.compare(".xz")) {
        
	  //    std::cout << "process : " << fileName << "\n";   
          kba::StreamThread st(fileName, dumpStream, &bscorer, lockMutex);
	  //        std::cout << "creating thread \n";
	  //  st.parseFile();
	  boost::thread *streamThread = new boost::thread(st, cutOffScore); 
	  aliveThreads.push_back(streamThread);
          threadGroup.add_thread(streamThread);
          index++;
        }
      }
    }
    
    if(index > 0) {
      threadGroup.join_all();
    } 
    delete lockMutex;
  
  }

  dumpStream->close(); 
  delete dumpStream;
  time_t endTime;
  time(&endTime);
  double seconds = difftime(endTime,startTime);
  std::cout << "Total time in seconds :: "<< seconds << "\n";
}

int main(int argc, char *argv[]){
  std::string taggerId;
  std::string corpusPath;
  std::string topicFile;
  std::string dirList;
  bool printStream = false;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help","the help message")
    ("file",cmndOp::value<std::string>(&corpusPath)->default_value("../help/corpus"),"the file or base dir to process" )
    ("efile",cmndOp::value<std::string>(&topicFile)->default_value("../help/topic.json"),"only process the directories in this list")
    ("dir-list",cmndOp::value<std::string>(),"Process the directories in this dir list only. this when I want to distribut my job over the nodes of ir server.")
    ("dfile",cmndOp::value<std::string>(),"The dump file")
    ("stream-id",cmndOp::value<std::string>(),"The stream id to search") 
    ("anchor"," print anchor text")
    ("title"," print title text")
    ("sentence"," print sentences ")
    ("repo",cmndOp::value<std::string>(), "Repository path of the rdfs store")
    ("repo-name", cmndOp::value<std::string>(), "The name of repository in the repository path") 
    ("print-model", "print the model")
    ("print-stream", cmndOp::value<bool>(&printStream)->default_value(false), "print the stream")
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
    if(cmndMap.count("equery")) {
      RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld());

      std::vector< boost::shared_ptr<unsigned char> > sources = rdfquery->getSourceNodes((unsigned char*)"http://dbpedia.org/unigramtoken",(unsigned char*)(cmndMap["equery"].as<std::string>().c_str()));
      for(int index=0; index < sources.size() ; index++) {
        unsigned char* node  = sources[index].get();
	std::cout << node << "\n";
      }
      //rdfparser->streamModel(stdout);  
    }
  }
  


  if(corpusPath.size() > 0  && topicFile.size() > 0 && cmndMap.count("dfile")) {
    std::string dumpFile = cmndMap["dfile"].as<std::string>();
    std::vector<std::string> directories; 
    if(cmndMap.count("dir-list")) {
      std::cout << "dir list set\n";
      std::ifstream inputFile(cmndMap["dir-list"].as<std::string>().c_str());
      for(std::string line;getline(inputFile, line);) {
        directories.push_back(line);
      }
      inputFile.close();
      std::sort(directories.begin(), directories.end(), compareString);
        
    } 
    performCCRTask(topicFile, corpusPath, dumpFile, directories);
  }
   

  if (!cmndMap.count("file") ) {
    std::cout <<  "no input file specified use --file \n";
    return -1;
  }
 
  if(!printStream || !cmndMap.count("stream-id")) {
    std::cout << "Neither print-stream -- nor stream-id option\n";
    return -1;
  }

  std::string basePath = cmndMap["file"].as<std::string>();
  bool isDirectory = indri::file::Path::isDirectory(basePath );   
  indri::file::FileTreeIterator files(basePath);
  if(!isDirectory) {
    if(cmndMap.count("stream-id")) {
      findStreamId(basePath, cmndMap["stream-id"].as<std::string>());
    }
    else if(printStream)
     iterateOnStream(basePath, cmndMap, taggerId);
  }
  else {
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      
      std::string fileName(*files);
      if(cmndMap.count("stream-id")) {
        findStreamId(fileName, cmndMap["stream-id"].as<std::string>());
      }
      else if(printStream)
        iterateOnStream(fileName, cmndMap, taggerId);
    }
  }
  return 0;
}
