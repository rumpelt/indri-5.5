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
#include "RelatedEntityScorer.hpp"
#include "BM25Scorer.hpp"
#include "DumpKbaResult.hpp"
#include "StreamThread.hpp"
#include "StreamIndex.hpp"
#include <boost/thread.hpp>
#include <time.h>
#include "Logging.hpp"
#include "TermDict.hpp"
#include "TimeConversion.hpp"

namespace cmndOp = boost::program_options;

std::vector<kba::entity::Entity*> ENTITY_SET;
std::unordered_set<std::string> STOP_SET;

bool compareString(std::string firstStr, std::string secondStr) {
  if(firstStr.compare(secondStr) < 0) 
    return true;
  return false;
}

void storeEvaluationData(std::string trgFile) {
  std::ifstream trgStream(trgFile.c_str());
  std::string dbName = "eval.db"; 
  StatDb* st = new StatDb();
  st->crtEvalDb("/usa/arao/test/eval.db", false);

  std::string row;
    while(std::getline(trgStream, row)) {
    if(row.find('#') == 0)
      continue;
    std::vector<std::string> rowTokens = Tokenize::split(row);
    
    kba::term::EvaluationData* evalData = new kba::term::EvaluationData();

    std::string stream_id = rowTokens.at(2);
    evalData->timeStamp =strtol(stream_id.substr(0, stream_id.find("-")).c_str(), NULL, 10);
    evalData->docId = stream_id.substr(stream_id.find("-") +1);
    std::cout << "eval data time " << evalData->timeStamp <<  " doc id "<< evalData->docId << "\n"; 
    evalData->topic = rowTokens.at(3);
    evalData->rating = strtol(rowTokens.at(5).c_str(), NULL, 10);
    evalData->directory = rowTokens.at(7);
    evalData->cleanVisibleSize = strtol(rowTokens.at(11).c_str(), NULL, 10);
    st->wrtEvalSt(evalData);
    delete evalData;
  }

  delete st;
  st = new StatDb();
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


void termStatIndexer(std::string entityfile, std::string pathToProcess, std::vector<std::string> dirList,   std::unordered_set<std::string> stopSet) {
  using namespace kba::term;
  std::map<std::string, std::string> repoMap;
  repoMap.insert(std::pair<std::string, std::string> ("wikiToDb","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("labels","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("internalentity","/usa/arao/dbpediadumps/dbpedia7bdb"));
   
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  kba::entity::populateEntityStruct(ENTITY_SET, repoMap);
  std::vector<kba::entity::Entity*> filterSet;
  for(std::vector<kba::entity::Entity*>::iterator entityIt = ENTITY_SET.begin(); entityIt != ENTITY_SET.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    if((entity->label).size() > 0)
      filterSet.push_back(entity);
    else
      delete entity; // I donot want to see that any more
  } 
  ENTITY_SET = filterSet;
  
  kba::term::CorpusStat* corpusStat = new kba::term::CorpusStat();
  std::set<TopicStat*> topicStatSet = kba::term::crtTopicStatSet(ENTITY_SET);
  std::set<TermStat*> termStatSet = kba::term::crtTermStatSet(ENTITY_SET, stopSet);
  std::map<TopicTermKey*, TopicTermValue*> topicTermMap = kba::term::crtTopicTermMap(ENTITY_SET, stopSet);


  std::cout << "TermSet size : " << termStatSet.size() << " topicTerm Size " << topicTermMap.size() << "\n";
  StatDb* stDb = new StatDb();

  stDb->crtTrmDb("/usa/arao/test/eff_imp_term-o.db", false);
  stDb->crtCrpDb("/usa/arao/test/eff_imp_corpus-o.db", false);
  
  std::unordered_set<std::string> termsToFetch;
  for(std::set<TermStat*>::iterator termIt = termStatSet.begin(); termIt != termStatSet.end(); ++termIt  ) {
    termsToFetch.insert((*termIt)->term);
  } 

  std::vector<std::string> dirBunch;
  std::sort(dirList.begin(), dirList.end(), compareString); 
  std::string prevDayDate = (dirList.at(0)).substr(0, dirList.at(0).rfind('-'));
  std::unordered_set<std::string> nullStopSet;
  kba::StreamIndex* streamIndex = new kba::StreamIndex(topicTermMap, corpusStat, topicStatSet, termStatSet, stDb, nullStopSet, termsToFetch);

  for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
    std::string dayDate = (*dirIt).substr(0, (*dirIt).rfind('-'));

    if(dayDate.compare(prevDayDate) != 0) {
      streamIndex->setCollectionTime(kba::time::convertDateToTime(prevDayDate));
      streamIndex->processDir(dirBunch);
      streamIndex->reset();
      dirBunch.clear();
      Logger::LOG_MSG("KbProcessing.cc", "termStatIndexer", "processed dirs :"+prevDayDate); 
    }
    dirBunch.push_back(pathToProcess+"/"+*dirIt);
    prevDayDate = dayDate;

  }

  if(dirBunch.size() > 0) {
    streamIndex->setCollectionTime(kba::time::convertDateToTime(prevDayDate));
    streamIndex->processDir(dirBunch);
    streamIndex->reset();
    Logger::LOG_MSG("KbProcessing.cc", "termStatIndexer", "processed dirs :"+prevDayDate); 
    dirBunch.clear();
    delete streamIndex;
  }
  delete streamIndex;
  delete stDb;
}
void performCCRTask(std::string entityfile, std::string pathToProcess, std::string fileToDump, std::vector<std::string> dirList, std::unordered_set<std::string> stopSet) {
  int cutOffScore=200; 
  std::map<std::string, std::string> repoMap;
  repoMap.insert(std::pair<std::string, std::string> ("wikiToDb","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("labels","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("internalentity","/usa/arao/dbpediadumps/dbpedia7bdb"));

   
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  kba::entity::populateEntityStruct(ENTITY_SET, repoMap);

  
  
  std::vector<kba::entity::Entity*> filterSet;
  
  for(std::vector<kba::entity::Entity*>::iterator entityIt = ENTITY_SET.begin(); entityIt != ENTITY_SET.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    
    if((entity->label).size() > 0) {
      std::cout << "Entity :" << entity->label << " ";
      for(std::vector<std::string>::iterator it = (entity->labelTokens).begin(); it != (entity->labelTokens).end(); ++it) {
	std::cout << *it << " ";
      }
      std::cout << "\n";
      filterSet.push_back(entity);
    }
    else
      delete entity;
  } 
  
  ENTITY_SET = filterSet;

  boost::shared_ptr<kba::term::TermBase> termBase(new kba::term::TermBase(ENTITY_SET));

  std::cout << "total enti : " << ENTITY_SET.size() << (ENTITY_SET[0])->wikiURL << "\n"; 
  bool isDirectory = indri::file::Path::isDirectory(pathToProcess );   
 
  //  kba::scorer::BaseLineScorer bscorer(ENTITY_SET); 
  kba::scorer::RelatedEntityScorer scorer(ENTITY_SET, repoMap);
  //kba::scorer::BM25Scorer scorer(ENTITY_SET, termBase);
  // kba::dump::writeHeader(fileToDump);
  std::fstream* dumpStream = new std::fstream(fileToDump.c_str(), std::fstream::out | std::fstream::app);
  time_t startTime;
  time(&startTime);

  if(!isDirectory) {
    std::string fileExtension = pathToProcess.substr(pathToProcess.size() - 3);
    if(!fileExtension.compare(".xz")) {
      kba::StreamThread st(pathToProcess, dumpStream, &scorer, 0, stopSet);
      //      st.setTermBase(termBase.get());
      st.parseFile(cutOffScore);
    }
  }
  else {
    int index = 0;
    boost::thread_group threadGroup;
    boost::mutex* lockMutex = new boost::mutex;
 
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
	  threadGroup.join_all();
	  aliveThreads.clear();
          index = 0;
        }
        
        std::string fileName(*files);
        std::string fileExtension = fileName.substr(fileName.size() - 3);
      
        if(!fileExtension.compare(".xz")) {
          kba::StreamThread st(fileName, dumpStream, &scorer, lockMutex, stopSet);
	  //  st.setTermBase(termBase.get());
	  //  st.parseFile();
	  boost::thread *streamThread = new boost::thread(st, cutOffScore); 
          
	  aliveThreads.push_back(streamThread);
          threadGroup.add_thread(streamThread);
          index++;
        }
      }
      Logger::LOG_MSG("KbaProcess.cc", "performCCRTask", "processed file :"+*pathIt);
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
  std::stringstream seconds ;
  seconds << difftime(endTime,startTime);
  
  Logger::LOG_MSG("KbaProcess.cc", "performCCRTask", " Total Time in seconds  :"+seconds.str()); 

}

int main(int argc, char *argv[]){
  std::string taggerId;
  std::string corpusPath;
  std::string topicFile;
  std::string dirList;
  std::string stopFile;
  std::string logFile;
  std::string trainingFile;
  std::string berkleyDbDir;
  char* tz;
  tz = getenv("TZ");
  if(tz) {
    std::cout << "Setting the timezone\n";
    setenv("TZ", "", 1);
    tzset();
  }
  
  bool printStream = false;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help","the help message")
    ("log", cmndOp::value<std::string>(&logFile), "Log file to write msg to")
    ("file",cmndOp::value<std::string>(&corpusPath)->default_value("../help/corpus"),"the file or base dir to process" )
    ("trng",cmndOp::value<std::string>(&trainingFile),"the trec kba judgement file, for this option to work bdb-dir should be specified" )
    ("stat", "Write term and corpus and topic stats")
    ("efile",cmndOp::value<std::string>(&topicFile)->default_value("../help/topic.json"),"only process the directories in this list")
    ("dir-list",cmndOp::value<std::string>(),"Process the directories in this dir list only. this when I want to distribut my job over the nodes of ir server.")
    ("dfile",cmndOp::value<std::string>(),"The dump file")
    ("stop-file,SF",cmndOp::value<std::string>(&stopFile)->default_value("../help/sql40stoplist"),"the file containing the stop words") 
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
  
  assert(logFile.size() > 0);  
  Logger::LOGGER(logFile);
  
  if(trainingFile.size() > 0) {
    storeEvaluationData(trainingFile);
  }

  STOP_SET  = Tokenize::getStopSet(stopFile);

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
  

  if (cmndMap.count("stat") > 0 && corpusPath.size() > 0 && topicFile.size()) {
    if(cmndMap.count("dir-list")) {
      std::vector<std::string> directories; 
      std::ifstream inputFile(cmndMap["dir-list"].as<std::string>().c_str());
      for(std::string line;getline(inputFile, line);) {
        directories.push_back(line);
      }
      inputFile.close();
      termStatIndexer(topicFile, corpusPath, directories, STOP_SET);
    }
  }

  if(corpusPath.size() > 0  && topicFile.size() > 0 && cmndMap.count("dfile")) {
    std::string dumpFile = cmndMap["dfile"].as<std::string>();
    std::vector<std::string> directories; 
    if(cmndMap.count("dir-list")) {
   
      std::ifstream inputFile(cmndMap["dir-list"].as<std::string>().c_str());
      for(std::string line;getline(inputFile, line);) {
        directories.push_back(line);
      }
      inputFile.close();
      std::sort(directories.begin(), directories.end(), compareString);
        
    } 
    performCCRTask(topicFile, corpusPath, dumpFile, directories, STOP_SET);
  }
   
  
  if (!cmndMap.count("file") ) {
    std::cout <<  "no input file specified use --file \n";
    Logger::CLOSE_LOGGER();
    if(tz) {
      setenv("TZ", tz, 1);
      tzset();
    }
    return -1;
  }
 
  if(!printStream && !cmndMap.count("stream-id")) {
    std::cout << "Neither print-stream -- nor stream-id option\n";
    Logger::CLOSE_LOGGER();
    if(tz) {
      setenv("TZ", tz, 1);
      tzset();
    }
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

  Logger::CLOSE_LOGGER();
  if(tz) {
      setenv("TZ", tz, 1);
      tzset();
  }
  return 0;
}
