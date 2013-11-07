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
#include "ParsedStream.hpp"
#include "StreamIndex.hpp"
#include <boost/thread.hpp>
#include <time.h>
#include "Logging.hpp"
#include "TermDict.hpp"
#include "TimeConversion.hpp"

namespace cmndOp = boost::program_options;

std::vector<kba::entity::Entity*> ENTITY_SET;
std::unordered_set<std::string> STOP_SET;
int16_t RATING_LEVEL = 2; // Just to work on vital documents

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

std::set<kba::term::TopicTerm*> readTermRelevance(std::string fileName) {
  std::ifstream tfile;
  tfile.open(fileName, std::ifstream::in);
  std::string topic;
  std::string term;
  time_t collectionTime;
  unsigned int judgedDocFreq;
  unsigned int relevantDocFreq;
  int relevantSetSize;
  std::set<kba::term::TopicTerm*> tpcTrm;
  while((tfile >> topic >> term >> collectionTime >> judgedDocFreq >> relevantDocFreq >> relevantSetSize)) {
    kba::term::TopicTerm* tpc = new TopicTerm(topic, term);
    tpc->collectionTime = collectionTime;
    tpc->judgedDocFreq = judgedDocFreq;
    tpc->relevantDocFreq = relevantDocFreq;
    tpc->relevantSetSize = relevantSetSize;
    //    std::cout << tpc->topic << " " << tpc->term << " " << tpc->collectionTime << " " << tpc->relevantDocFreq << "\n";
    tpcTrm.insert(tpc);    
  }

  tfile.close();
  return tpcTrm;
}

std::set<kba::term::TopicTerm*>& getTermRelevance(std::vector<std::string> dirList, std::set<kba::term::TopicTerm*>& trmTpc, std::string fName) {
  using namespace kba::term;
  using namespace boost;
  std::unordered_set<std::string> dummySet;
 
  std::set<std::string> foundIds;

  std::sort(dirList.begin(), dirList.end(), compareString); 
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  streamcorpus::StreamItem* streamItem = 0;
  StatDb st;
  st.crtEvalDb("/usa/arao/test/eval.db", true);
  std::set<std::string> docIds = st.getEvalDocIds();
  std::cout << "Num docs " << docIds.size() << "\n";
  for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
    std::string dayDate = (*dirIt).substr(0, (*dirIt).rfind('-'));
    std::string pathToProcess = "../help/corpus/" + *dirIt;
    
    indri::file::FileTreeIterator files(pathToProcess);
    //    std::cout << "Processing dir " << pathToProcess << "\n";    
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      tdextractor->open(*files);
      while((streamItem = tdextractor->nextStreamItem()) != 0) {
        
	std::string stream_id = streamItem->stream_id;
        if(docIds.find(stream_id) == docIds.end() ||  foundIds.find(stream_id) != foundIds.end())
          continue;

        int dtSize = (streamItem->body).clean_visible.size();
        if (dtSize <= 0)
          continue;
        
	std::vector<shared_ptr<EvaluationData> > evDts = st.getEvalData(stream_id, false, true);     
	//	std::cout << "Streaim id " << stream_id << " "<<evDts.size() << " " << ev->rating << "\n";   
        if(evDts.size() <= 0)
          continue;
        int16_t minRating = 4;
        time_t collectionTime  =-1;
	std::string topic;   

        for(std::vector<shared_ptr<EvaluationData> >::iterator evIt = evDts.begin(); evIt != evDts.end(); ++evIt) {
          EvaluationData* ev = (*evIt).get();
	  //	  std::cout << "Ev data " << ev->topic << " docsize " << ev->cleanVisibleSize << " doc size " << dtSize << " " << ev->rating <<"\n";
          if(ev->cleanVisibleSize == dtSize) {
            if(ev->rating  < minRating) {
              minRating = ev->rating;
	      collectionTime = ev->timeStamp; // actuall no need to save this all the evaluation data have same timestamp which is same as in the stream_id.
              topic = ev->topic;
	    }
	  }
	}

        if(minRating >= -1 && minRating <= 2) {
          foundIds.insert(stream_id);
	  kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createLightParsedStream(streamItem, dummySet);
          for(std::set<TopicTerm*>::iterator trmTpcIt = trmTpc.begin(); trmTpcIt != trmTpc.end(); ++trmTpcIt) {
            TopicTerm* tt =  *trmTpcIt;
	    if(!(tt->topic).compare(topic)) {
              tt->collectionTime = collectionTime;
              tt->relevantSetSize++;
	      //   std::cout << "Updated releant set sie " << topic << " "  << tt->relevantSetSize  << " " <<tt->collectionTime <<"\n";
              if((parsedStream->tokenSet).find(tt->term) != (parsedStream->tokenSet).end()) {
                tt->judgedDocFreq += 1;
		//              std::cout << "Updated judged doc freq " << topic << " " << tt->judgedDocFreq << "\n";
                if(minRating >= RATING_LEVEL) 
                  tt->relevantDocFreq += 1; 
	      }
	    }
	  }
          delete parsedStream;
	}                  	
      }   
      tdextractor->reset();
    }
    Logger::LOG_MSG("KbaProcessing.cc","getTermRelevance", "process dir" +*dirIt);
  }

  std::ofstream relStream(fName, std::ofstream::trunc| std::ofstream::out);
  for (std::set<TopicTerm*>::iterator termIt = trmTpc.begin(); termIt != trmTpc.end(); ++termIt) {
    TopicTerm* tt = *termIt;
    relStream << (tt)->topic << " " << (tt)->term << " " << tt->collectionTime  << " " << tt->judgedDocFreq << "  " << tt->relevantDocFreq << " " << tt->relevantSetSize <<"\n";
  }
  relStream.close();
  delete tdextractor;  
  return trmTpc;
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
  kba::entity::populateEntityStruct(ENTITY_SET, repoMap, STOP_SET);
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
  std::set<TopicStat> topicStatSet = kba::term::crtTopicStatSet(ENTITY_SET);
  std::set<TermStat*> termStatSet = kba::term::crtTermStatSet(ENTITY_SET, stopSet); // I m not free this atthe end so there is a leak
  std::map<TopicTermKey, TopicTermValue> topicTermMap;
  std::set<TopicTerm*> topicTerm = kba::term::crtTopicTerm(ENTITY_SET);
  
  // topicTerm = readTermRelevance("/usa/arao/.Trash/TermRelevance.csv");
  //  std::cout << topicTerm.size() << "\n";
  //  std::string fname = "../help/FastTermRelevance.csv";
  //getTermRelevance(dirList,topicTerm, fname);
  //return;
  std::cout << "TermSet size : " << termStatSet.size() << " topicTerm Size " << topicTermMap.size() << "\n";
  StatDb* stDb = new StatDb();

  stDb->crtTrmDb("/usa/arao/test/term.db", true);
  stDb->crtCrpDb("/usa/arao/test/corpus.db", true);
  
  std::set<std::string> termsToFetch;
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
  using namespace kba::term;
  std::map<std::string, std::string> repoMap;
  repoMap.insert(std::pair<std::string, std::string> ("wikiToDb","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("labels","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("internalentity","/usa/arao/dbpediadumps/dbpedia7bdb"));

   
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  kba::entity::populateEntityStruct(ENTITY_SET, repoMap, STOP_SET);
  std::vector<kba::entity::Entity*> filterSet;

  for(std::vector<kba::entity::Entity*>::iterator entityIt = ENTITY_SET.begin(); entityIt != ENTITY_SET.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    
    if((entity->label).size() > 0) {
      filterSet.push_back(entity);
    }
    else
      delete entity;
  } 
  
  ENTITY_SET = filterSet;
   
  std::cout << "total enti : " << ENTITY_SET.size() << (ENTITY_SET[0])->wikiURL << "\n"; 
  bool isDirectory = indri::file::Path::isDirectory(pathToProcess );   
  
  kba::term::CorpusStat* corpusStat = new kba::term::CorpusStat();
  std::set<TermStat*> termStatSet = kba::term::crtTermStatSet(ENTITY_SET, STOP_SET); // I m not free this atthe end so there is a leak
  std::set<TopicTerm*> topicTerm = kba::term::crtTopicTerm(ENTITY_SET);
  std::set<std::string> termSet;
  for(std::set<TermStat*>::iterator termIt = termStatSet.begin(); termIt != termStatSet.end(); ++termIt) {
    termSet.insert((*termIt)->term);
  }

  std::fstream* dumpStream = new std::fstream(fileToDump.c_str(), std::fstream::out | std::fstream::app);
  time_t startTime;
  time(&startTime);

  if(!isDirectory) {
    std::string fileExtension = pathToProcess.substr(pathToProcess.size() - 3);
    if(!fileExtension.compare(".xz")) {
    }
  }
  else {
    std::vector<std::string> dirBunch;
    short countPass = 0;
    bool firstPass = true;
    std::string prevDayDate = (dirList.at(0)).substr(0, (dirList.at(0)).rfind('-'));
    for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
      std::string dayDate = (*dirIt).substr(0, (*dirIt).rfind('-'));
       if(dayDate.compare(prevDayDate) != 0) {
	 kba::StreamThread* st = new kba::StreamThread(dirBunch, dumpStream, ENTITY_SET, STOP_SET);
         st->setTermStat(termStatSet);
         st->setCorpusStat(corpusStat);   
         st->setTopicTerm(topicTerm);
         st->setTermSet(termSet);
         st->spawnParserNScorers(firstPass);
	 Logger::LOG_MSG("KbaProcess.cc","performCCRTask", "finished processing "+prevDayDate);
         ++countPass;
         if(firstPass && countPass >= 7)
           firstPass = false;
         dirBunch.clear();
      }
      dirBunch.push_back(pathToProcess+"/"+*dirIt);
      prevDayDate = dayDate;
    }
    if(dirBunch.size() > 0) {
      kba::StreamThread* st = new kba::StreamThread(dirBunch, dumpStream, ENTITY_SET, STOP_SET);
       st->setTermStat(termStatSet);
       st->setCorpusStat(corpusStat);   
       st->setTopicTerm(topicTerm);
       st->setTermSet(termSet); 
       st->spawnParserNScorers(firstPass);
       Logger::LOG_MSG("KbaProcess.cc","performCCRTask", "finished processing "+prevDayDate);
       dirBunch.clear();
    }
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
      for(size_t index=0; index < sources.size() ; index++) {
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
