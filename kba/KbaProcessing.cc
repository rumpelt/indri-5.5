#include "QueryThread.hpp"
#include <indri/Parameters.hpp>

#include <dirent.h>
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
#include "RelatedEntityScorer.hpp"
#include "BM25Scorer.hpp"
#include "DumpKbaResult.hpp"
#include "StreamThread.hpp"
#include "ParsedStream.hpp"
#include "Passage.hpp"
#include "StreamIndex.hpp"
#include <boost/thread.hpp>
#include <time.h>
#include "Logging.hpp"
#include "TermDict.hpp"
#include "TimeConversion.hpp"
#include "FilterThread.hpp"
#include <assert.h>
#include <ctype.h>

namespace cmndOp = boost::program_options;

std::vector<kba::entity::Entity*> ENTITY_SET;
std::unordered_set<std::string> STOP_SET;
int16_t RATING_LEVEL = 2; // Just to work on vital documents


bool compareString(std::string firstStr, std::string secondStr) {
  if(firstStr.compare(secondStr) < 0) 
    return true;
  return false;
}

struct TruthData {
  std::string topic;
  std::string streamid;
  std::string rating;
  std::string directory;
};

/**
 * Sorts the data on the directory information i.e. sorted on the day of the corpus
 */
bool compareTruthData(TruthData tdone, TruthData tdtwo) {
  if( tdone.directory.compare(tdtwo.directory) < 0)
    return true;

  return false;
}

/**
 * List of positive entities which do not have  
 */
std::set<std::string> noPositiveJudgement(std::string noPositiveFile) {
  std::ifstream flStream(noPositiveFile.c_str());
  std::set<std::string> entityList;
  std::string entityName;
  while((flStream  >> entityName)) {
    entityList.insert(entityName);
  }
  flStream.close();
  return entityList;
   
}

void HighRecallInfo(std::string recallFile) {
  std::ifstream trgStream(recallFile.c_str());
  StatDb* st = new StatDb();
  st->crtStreamDb("/usa/arao/test/streamid-small.db", false);
  std::string row;
  while(std::getline(trgStream, row)) {
    std::vector<std::string> rowTokens = Tokenize::split(row);
    int score = strtol(rowTokens.at(4).c_str(), NULL, 10);
    if (score > 300) {
      kba::term::StreamInfo stInfo;
   
      std::string stream_id = rowTokens.at(2);
      stInfo.sTime = strtol(stream_id.substr(0, stream_id.find("-")).c_str(), NULL, 10);
    
      stInfo.docId = stream_id.substr(stream_id.find("-") +1);
      stInfo.directory = rowTokens.at(7);

      st->wrtStreamInfo(stInfo);
    }
  }
  st->closeStreamDb();
  Logger::LOG_MSG("KbaProcessing.cc", "HighRecallInfo", " finished writing the streamdbs ");
  delete st;
  trgStream.close();
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
  trgStream.close();
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
	std::vector<boost::shared_ptr<EvaluationData> > evDts = st.getEvalData(stream_id, false, true);     
	//	std::cout << "Streaim id " << stream_id << " "<<evDts.size() << " " << ev->rating << "\n";   
        if(evDts.size() <= 0)
          continue;
        int16_t minRating = 4;
        time_t collectionTime  =-1;
	std::string topic;   

        for(std::vector<boost::shared_ptr<EvaluationData> >::iterator evIt = evDts.begin(); evIt != evDts.end(); ++evIt) {
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

std::vector<kba::entity::Entity*> getEntityList(std::string entityFile, std::string noPositiveFile, bool useDbpedia) {
  std::vector<kba::entity::Entity*> entityList;
  kba::entity::populateEntityList(entityList, entityFile);
  if(useDbpedia) {
    std::map<std::string, std::string> repoMap;
    repoMap.insert(std::pair<std::string, std::string> ("wikiToDb","/usa/arao/dbpediadumps/dbpedia7bdb"));
    repoMap.insert(std::pair<std::string, std::string> ("labels","/usa/arao/dbpediadumps/dbpedia7bdb"));
    repoMap.insert(std::pair<std::string, std::string> ("internalentity","/usa/arao/dbpediadumps/dbpedia7bdb"));
    repoMap.insert(std::pair<std::string, std::string> ("abstract","/usa/arao/dbpediadumps/dbpedia7bdb"));
    kba::entity::populateEntityStruct(entityList, repoMap, STOP_SET);
    kba::entity::updateEntityWithAbstract(entityList, repoMap.at("abstract"), "abstract", STOP_SET );
  }
  
  std::vector<kba::entity::Entity*> filterSet;
  std::set<std::string> noPositive = noPositiveJudgement(noPositiveFile);
  for(std::vector<kba::entity::Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    /**
    if((entity->label).size() <= 0) {
      std::cout << "Could not get the label for " << entity->wikiURL << "\n";
    }
    else
      std::cout << entity->label << "\n";
    */
    if( noPositive.find(entity->wikiURL) == noPositive.end() && (entity->label).size() > 0) {
      filterSet.push_back(entity);
    }
    else
      delete entity;
  } 
  return filterSet;
}

void performCCRTask(std::string entityfile, std::string pathToProcess, std::string fileToDump, std::vector<std::string> dirList, std::string noPositiveFile, std::unordered_set<std::string> stopSet) {
  using namespace kba::term;
  std::map<std::string, std::string> repoMap;
  repoMap.insert(std::pair<std::string, std::string> ("wikiToDb","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("labels","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("internalentity","/usa/arao/dbpediadumps/dbpedia7bdb"));
  repoMap.insert(std::pair<std::string, std::string> ("abstract","/usa/arao/dbpediadumps/dbpedia7bdb"));
   
  kba::entity::populateEntityList(ENTITY_SET, entityfile);
  kba::entity::populateEntityStruct(ENTITY_SET, repoMap, STOP_SET);
  
  std::vector<kba::entity::Entity*> filterSet;
  std::set<std::string> noPositive = noPositiveJudgement(noPositiveFile);
  for(std::vector<kba::entity::Entity*>::iterator entityIt = ENTITY_SET.begin(); entityIt != ENTITY_SET.end(); entityIt++) {
    kba::entity::Entity* entity =  *entityIt;
    if( noPositive.find(entity->wikiURL) == noPositive.end() && (entity->label).size() > 0) {
      filterSet.push_back(entity);
    }
    else
      delete entity;
  } 


  kba::entity::updateEntityWithAbstract(filterSet, repoMap.at("abstract"), "abstract", STOP_SET ); 
 
  //  std::cout << "total enti : " << filterSet.size() << "\n"; 
  bool isDirectory = indri::file::Path::isDirectory(pathToProcess );   
  
  kba::term::CorpusStat* corpusStat = new kba::term::CorpusStat();
  std::set<TermStat*> termStatSet = kba::term::crtTermStatSet(filterSet, STOP_SET); // I m not free this atthe end so there is a leak
 
  std::set<TopicTerm*> topicTerm = kba::term::crtTopicTerm(filterSet);
  std::set<std::string> termSet;
  std::map<std::string, TermStat*> termStatMap;
  for(std::set<TermStat*>::iterator termIt = termStatSet.begin(); termIt != termStatSet.end(); ++termIt) {
    termSet.insert((*termIt)->term);
    termStatMap.insert(std::pair<std::string, TermStat*>((*termIt)->term, *termIt));
  }
  
  //  std::cout << "TermStat set size " << termSet.size() << " " << termStatMap.size() << "\n";
 
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
    //std::cout << "Prev Day date " << prevDayDate << " " << dirList.at(0) << "\n";
    StatDb stDb;
    //stDb.crtStreamDb(std::string("/usa/arao/test/streamid.db"), true);
    stDb.crtTrmDb(std::string("/usa/arao/test/term-")+prevDayDate, false);
    stDb.crtCrpDb(std::string("/usa/arao/test/corpus-")+prevDayDate, false);

    for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
      std::string dayDate = (*dirIt).substr(0, (*dirIt).rfind('-'));
       if(dayDate.compare(prevDayDate) != 0) {
	 //   std::cout << "Day date " << prevDayDate << " size " << dirBunch.size() << "\n";
	 kba::StreamThread* st = new kba::StreamThread(dirBunch, dumpStream, filterSet, STOP_SET, prevDayDate);
         st->setTermStat(termStatMap);
         st->setCorpusStat(corpusStat);   
         st->setTermSet(termSet);
         st->setStatDb(&stDb);
         st->spawnParserNScorers(firstPass);
	 Logger::LOG_MSG("KbaProcess.cc","performCCRTask", "finished processing "+prevDayDate);
         ++countPass;
         if(countPass >= 13)
           firstPass = false;
         dirBunch.clear();
      }
      dirBunch.push_back(pathToProcess+"/"+*dirIt);
      prevDayDate = dayDate;
    }
    if(dirBunch.size() > 0) {
      firstPass = false;
      kba::StreamThread* st = new kba::StreamThread(dirBunch, dumpStream, filterSet, STOP_SET, prevDayDate);
       st->setTermStat(termStatMap);
       st->setCorpusStat(corpusStat);   
       st->setTermSet(termSet);
       st->setStatDb(&stDb); 
       st->spawnParserNScorers(firstPass);
       Logger::LOG_MSG("KbaProcess.cc","performCCRTask", "finished processing "+prevDayDate);
       dirBunch.clear();
    }
    stDb.closeStreamDb();
    stDb.closeTrmDb();
    stDb.closeCrpDb();
  }

  dumpStream->close(); 
  delete dumpStream;
  time_t endTime;
  time(&endTime);
  std::stringstream seconds ;
  seconds << difftime(endTime,startTime);
  
  Logger::LOG_MSG("KbaProcess.cc", "performCCRTask", " Total Time in seconds  :"+seconds.str()); 

}

std::string sanitizeQueryText(std::string& text) {
  std::stringstream ss;
  //  assert(text.size() < 256);
  //  std::cout << "Text size " << text.size() << std::endl;
  for(size_t index=0; index < text.size(); index++) {
    char c = text.at(index);
    //    if(c == ',' || c == '.' || c == '#' || c == '(' || c == ')' || c == '<' || c == '>' || c == '[' || c == ']' || c == '-' || c == '_' || c == '\'' || c== '%' || c == ':')
    //  continue;
    if(isalnum(c) || isspace(c) || isblank(c))
      ss << c;
  }

  return ss.str();
}

void processFilterThread(std::map<std::string, query_t*>& qMap, std::vector<std::string> dirList, std::vector<std::string> paramFiles, std::string dumpFile, std::map<std::string, kba::term::TermStat*> termStatMap, kba::term::CorpusStat* corpusStat) {
  //std::cout << " Number of dir " << dirList.size() << std::endl;
  std::string prevDayDate = dirList.at(0); //(dirList.at(0)).substr(0, (dirList.at(0)).rfind('-'));
  //std::cout << "prev Day " << prevDayDate << std::endl;
  std::vector<std::string> dirBunch;
  std::string runId = "test";

  //  indri::api::Parameters params = indri::api::Parameters::instance();
  indri::api::Parameters* params = new indri::api::Parameters();

  for(std::vector<std::string>::iterator paramIt = paramFiles.begin(); paramIt != paramFiles.end(); ++paramIt) {
    FILE* pFile = fopen((*paramIt).c_str(), "rb");
    if(pFile != NULL) {
      //      std::cout << "Loading file " << *paramIt << "\n";
      params->loadFile(*paramIt);
      fclose(pFile);
    }
  }
  std::vector<std::string> oldIndexDir ;//  we will be collecting index statistics from previous day indexs
  std::string oldestDir;
  int numHistoryDays = 5;
  for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.begin() + numHistoryDays ; ++dirIt) {
    oldIndexDir.push_back(*dirIt);
  }

  QueryThread qt(*params, oldIndexDir);
  FilterThread::dumpStream.open(dumpFile.c_str(), std::fstream::out | std::fstream::app); 

  for(std::vector<std::string>::iterator dirIt = dirList.begin() + numHistoryDays; dirIt != dirList.end(); ++dirIt) {
    std::string dir = *dirIt;
    FilterThread ft(*params, dir, qMap, corpusStat, termStatMap); //(prevDayDate, qMap, dumpFile, runId);
    ft._dumpFile = dumpFile;
    ft._runId = runId;
    //      ft.setParamFile(paramFiles);
    //ft.process(qt);
    ft.dumpDayStat(qt);
    //ft.expectationMaxim(qt);
    std::string oldest = oldIndexDir.front();
    oldIndexDir.erase(oldIndexDir.begin());
    oldIndexDir.push_back(dir);
    qt.removeDir(oldest);
    qt.addIndexDir(dir);
  }

  FilterThread::dumpStream.close();
  delete params;
} 

/**
 * truthVector is sorted on the directory information. eg. of directory : 2012-10-21-11
 * dir : is the the index dir representing a day of index e.g. of content : 2012-10-21
 * Returns the minimum rating for  the streamid and topic, else empty string if there is not rating.
 */
std::string returnRating(const std::string& topic, const std::string& streamId, const std::string& dir, std::vector<TruthData>& truthVector) {
  std::string minRating;
  int rating = 10;
  for(std::vector<TruthData>::iterator tvIt = truthVector.begin(); tvIt != truthVector.end(); ++tvIt) {
    TruthData td = *tvIt;
    std::string origDir = td.directory.substr(0, td.directory.rfind('-')); 
   
    int cmp = dir.compare(origDir);
    //    std::cout << origDir << std::endl;
    if(cmp == 0) {
      if(topic.compare(td.topic) == 0 && streamId.compare(td.streamid) == 0) {
        int curRating = atoi(td.rating.c_str());
        if(curRating < rating) {
          rating = curRating;
          minRating = td.rating;
	} 
      }
    }
    else if(cmp < 0)
      break;
  }
  return minRating;
}

std::vector<TruthData> parseTruthFile(std::string fileName) {
  std::ifstream truthFile(fileName.c_str());
  std::string line;
  std::vector<TruthData> truthVector;
  while(std::getline(truthFile,line)) {
    if (line[0] == '#')
      continue;
    //    std::cout << line << std::endl;
    std::vector<std::string> tokens = Tokenize::split(line);
    TruthData td;
    td.streamid = tokens[2];
    td.topic  = tokens[3];
    td.rating = tokens[5];
    td.directory = tokens[7];
    truthVector.push_back(td);
    //std::cout << "Pushing truth " << td.streamid << " " << td.topic << " " << td.rating << " " << td.directory << std::endl;
  }
  truthFile.close();
  std::sort(truthVector.begin(), truthVector.end(), compareTruthData);
  return truthVector;
}

std::map<std::string, Passage> getPassages(std::string dir, std::set<std::string>& streamIds) {
  std::map<std::string, Passage> psgMap;
  indri::file::FileTreeIterator files(dir);
  //std::cout << "Parsing Directoru "<< dir << std::endl;
  for(; files != indri::file::FileTreeIterator::end() ;files++) {
    std::string fName = *files;
    kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
    tdextractor->open(fName);
    streamcorpus::StreamItem* streamItem = 0;
    while((streamItem = tdextractor->nextStreamItem()) != 0) {
      std::string docId  = streamItem->stream_id;
      if(streamIds.find(docId) == streamIds.end() && (streamItem->body).clean_visible.size() <= 0) 
        continue;

        std::string title = streamcorpus::utils::getTitle(*streamItem);
        std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
        std::string body = (streamItem->body).clean_visible;
        std::string fullContent = title + " "+  anchor + " " + body;  
	std::vector<std::string> tokens = Tokenize::whiteSpaceSplit(fullContent, STOP_SET, true, 1, true);
        Passage psg;
        psg.setTrecId(docId);
        psg.setTerms(tokens);
        //psg.crtTermFreq();
        psgMap[docId] = psg;
    }
    delete tdextractor;
  }
  //std::cout << "End of parsing " << std::endl;
  return psgMap;
}

void loadQueryDocumentFromFile(std::map<std::string, query_t*>& queryMap, std::string fileName, std::string rating) {
  std::ifstream infile(fileName.c_str());
  std::string line;
  long lineNo = 1;
  query_t* query= 0;
  std::string trecId;
  while(std::getline(infile, line)) {
    std::vector<std::string> tokens = Tokenize::split(line);
    if((lineNo % 2) == 1) {
      std::string curRating = tokens[2];
      if(curRating.compare(rating) == 0) {
        std::string topicId = tokens[0];
        query = queryMap[topicId];
        trecId = tokens[1];
      }
      else
        query = 0;
    }
    else {
      if(query != 0) {
        bool alreadyPushed = false;
        for(std::vector<Passage>::iterator psgIt = query->relevantDocs.begin(); psgIt != query->relevantDocs.end(); ++psgIt) {
        Passage passage = *psgIt; 
        if (passage.getTrecId().compare(trecId) == 0)
          alreadyPushed = true;
	}
        if (alreadyPushed)
          continue;
        Passage psg;
        psg.setTerms(tokens);
        psg.setTrecId(trecId);
        psg.crtTermFreq();
        query->relevantDocs.push_back(psg);
      }
    }
  }
  infile.close();
  
}

void loadQueryDocuments(std::map<std::string, query_t*>& queryMap) {
  std::ofstream outfile("../help/VitalQueryDocuments", std::ofstream::out);
  std::string testTruthFile = "/usa/arao/trec/trec-kba/2013/evaluation/kba-scorer/data/trec-kba-ccr-judgments-2013-09-26-expanded-with-ssf-inferred-vitals-plus-len-clean_visible.before-and-after-cutoff.filter-run.txt";
  std::vector<TruthData> truth = parseTruthFile(testTruthFile);
  //std::cout << "Rating " << returnRating(topic, streamId, dirc, truthVector) << std::endl;

  std::map<std::string, Passage> psgMap;
  std::set<std::string> streamToGet;
  std::string stopDir = "2012-03-01-00";

  for(std::vector<TruthData>::iterator dtIt = truth.begin();  dtIt != truth.end(); ++dtIt) {
    TruthData tdata = *dtIt;
    if(tdata.directory.compare(stopDir) >= 0)
      continue;
    if(tdata.rating.compare("2") == 0)
      streamToGet.insert(tdata.streamid);
  }

  std::map<std::string, Passage> strmPsgMap;
  std::string prevDir = "";
  for(int idx = 0 ; idx < truth.size(); ++idx) {
    TruthData td = truth[idx];
    if(td.directory.compare(stopDir) >= 0 || td.rating.compare("2") != 0) {
      continue;
    }
    if(td.directory.compare(prevDir) != 0) {
      strmPsgMap.clear();
      std::string corpus = "../help/corpus/"+td.directory;
      strmPsgMap = getPassages(corpus, streamToGet);
      prevDir = td.directory;
    }
    Passage psg;
    query_t* query = 0; 
    try {
      psg = strmPsgMap.at(td.streamid); 
      query = queryMap.at(td.topic);
    } catch  (std::out_of_range& expt){
      continue;
    }
    bool alreadyPushed = false;
    for(std::vector<Passage>::iterator psgIt = query->relevantDocs.begin(); psgIt != query->relevantDocs.end(); ++psgIt) {
      Passage passage = *psgIt; 
      if (passage.getTrecId().compare(td.streamid) == 0)
        alreadyPushed = true;
    }
    if (alreadyPushed)
      continue;
    //std::cout << "Adding Psg to " << query->id << psg.getTerms().size() << std::endl;
    outfile << td.topic <<  " " << td.streamid << " " << td.rating << " " << td.directory << std::endl;
    std::vector<std::string> terms = psg.getTerms();  
    for(std::vector<std::string>::iterator vecIt = terms.begin(); vecIt != terms.end(); ++vecIt) {
      outfile << *vecIt << " ";
    }
    outfile << std::endl;
    query->relevantDocs.push_back(psg);
    prevDir =  td.directory; 
  }  
  //  outfile << "Done";
  outfile.close();
}

/**
 * We just populate the query struct
 */
void makeQuery(std::map<std::string, query_t*>& queryMap, std::vector<kba::entity::Entity*> entities) {
  int idx = 1;
  std::string queryType  = "indri";
  for(std::vector<kba::entity::Entity*>::iterator entIt = entities.begin(); entIt != entities.end() ; ++entIt, ++idx) {
    kba::entity::Entity* entity = *entIt;  
    std::string qtext = sanitizeQueryText(entity->label); // We sanitize because if we use the Indri Run Query then indri crashes due to invalid characters in the text.
    std::transform(qtext.begin(), qtext.end(), qtext.begin(), ::tolower);
    query_t* query = new query_t(idx, entity->wikiURL, qtext, queryType);
    query->textVector = Tokenize::split(query->text);
    if((entity->abstract).size() > 0) {
  
      std::string s_abstract = sanitizeQueryText(entity->abstract);
      std::transform(s_abstract.begin(), s_abstract.end(), s_abstract.begin(), ::tolower);
      query->description = s_abstract;
      //query->textVector = Tokenize::whiteSpaceSplit(s_abstract, STOP_SET, true, 1,true);
    }
    //else
    //  continue;
    
    queryMap.insert(std::pair<std::string, query_t*>(entity->wikiURL, query));
  }
}

void createDictionary(std::map<std::string, query_t*>& queryMap) {
}

std::map<std::string, query_t*> bootStrapIndri(std::map<std::string, query_t*>& queryMap, std::vector<std::string> dirList, std::vector<std::string> paramFiles, std::vector<kba::entity::Entity*> entities, int indexDirSize , std::map<std::string, kba::term::TermStat*>& termStatMap, kba::term::CorpusStat* corpusStat) {
  using namespace kba::term;
  std::vector<std::string> stopwords;
   indri::api::Parameters param = indri::api::Parameters::instance();
  //  indri::api::Parameters* param = new indri::api::Parameters();
  for(std::vector<std::string>::iterator paramIt = paramFiles.begin(); paramIt != paramFiles.end(); ++paramIt) {
    FILE* pFile = fopen((*paramIt).c_str(), "rb");
    if(pFile != NULL) {
      param.loadFile(*paramIt);
      fclose(pFile);
    }
    else
      std::cout << "Failed to load Parameter file " <<std::endl;
  }
  
  copy_parameters_to_string_vector( stopwords, param, "stopper.word" );
  for(std::vector<std::string>::iterator wordIt = stopwords.begin(); wordIt != stopwords.end(); ++wordIt)
      STOP_SET.insert(*wordIt);


  std::string queryType = "indri";
  int idx = 1;
  for(std::vector<kba::entity::Entity*>::iterator entIt = entities.begin(); entIt != entities.end() ; ++entIt, ++idx) {
    kba::entity::Entity* entity = *entIt;  
    std::string label = sanitizeQueryText(entity->label);   
        
    std::string qtext = "#weight( 3.0 ";
    qtext = qtext + "#combine( "+ label + ")";
    /**
    if((entity->abstract).size() > 0) {
      
      std::string s_abstract = sanitizeQueryText(entity->abstract);
      qtext = qtext + " 1.0 #combine( " + s_abstract + ")";
      label = label + entity->abstract;
    }
    */
    qtext = qtext + ")";

    //  std::cout << entity->wikiURL << " query " << qtext << "\n";
    query_t* query = new query_t(idx, entity->wikiURL, qtext, queryType);
    query->textVector = Tokenize::whiteSpaceSplit(label, STOP_SET, true, 1,   true);
    for(std::vector<std::string>::iterator wordIt = (query->textVector).begin(); wordIt != (query->textVector).end(); ++wordIt) {
      std::string word = *wordIt;
      if(termStatMap.find(word) == termStatMap.end()) {
	kba::term::TermStat* stat = new kba::term::TermStat();
        termStatMap.insert(std::pair<std::string, kba::term::TermStat*>(word, stat)); 
      }
    }
    queryMap.insert(std::pair<std::string, query_t*>(entity->wikiURL, query));
  }

  std::vector<std::string> indexDir;
  for(std::vector<std::string>::iterator dirIt = dirList.begin(); dirIt != dirList.end(); ++dirIt) {
    if(indexDir.size() >= indexDirSize) {
      std::cout << "index dir size " << indexDir.size() << std::endl;
      QueryThread* qt = new QueryThread(param, indexDir);
     
      for(std::map<std::string, query_t*>::iterator qMapIt = queryMap.begin(); qMapIt != queryMap.end(); ++qMapIt) {
        query_t* q = qMapIt->second;
        qt->_runQuery(q, true, 5, 10);
        q->addDocs(qt->getDocumentVector());       
	std::cout << q->id << " Expanded Text " << q->expandedText << "\n";
      }
     
      for(std::map<std::string, TermStat*>::iterator mapIt = termStatMap.begin(); mapIt != termStatMap.end(); ++mapIt) {
	std::string term = mapIt->first;
	kba::term::TermStat* termStat = mapIt->second;
        termStat->docFreq += qt->documentCount(term);
        termStat->collFreq += qt->termCount(term);
      }

      corpusStat->totalDocs += qt->documentCount();
      corpusStat->collectionSize += qt->termCount();

      delete qt;

      indexDir.clear();
    }
    indexDir.push_back(*dirIt);
  }

  if(indexDir.size() > 0) {
    std::cout << "index dir size " << indexDir.size() << std::endl;
    QueryThread* qt = new QueryThread(param, indexDir);
 
    for(std::map<std::string, query_t*>::iterator qMapIt = queryMap.begin(); qMapIt != queryMap.end(); ++qMapIt) {
      query_t* q = qMapIt->second;
      qt->_runQuery(q, true, 5, 10);
      q->addDocs(qt->getDocumentVector());       
      std::cout << q->id << " Expanded Text " << q->expandedText << "\n";
    }

    for(std::map<std::string, TermStat*>::iterator mapIt = termStatMap.begin(); mapIt != termStatMap.end(); ++mapIt) {
	std::string term = mapIt->first;
	kba::term::TermStat* termStat = mapIt->second;
        termStat->docFreq += qt->documentCount(term);
        termStat->collFreq += qt->termCount(term);
      }

      corpusStat->totalDocs += qt->documentCount();
      corpusStat->collectionSize += qt->termCount();


    delete qt;
  }
 
  return queryMap;
}

void prepareForCCR() {
}

int main(int argc, char *argv[]){
  std::string taggerId;
  std::string corpusPath;
  std::string topicFile;
  std::string dirList;
  std::string processList;
  std::string stopFile;
  std::string logFile;
  std::string trainingFile;
  std::string berkleyDbDir;
  std::string noPositiveFile;
  std::string dumpFile;
  std::vector<std::string> paramFiles;
  std::string baseIndexPath;

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
    ("require-positive",cmndOp::value<std::string>(&noPositiveFile)->default_value("../help/nopositive-judgementfile"),"Only process entities for which we have judgement")
    ("dir-list",cmndOp::value<std::string>(&dirList),"Process the directories in this dir list only. this when I want to distribut my job over the nodes of ir server.")
    ("run-list",cmndOp::value<std::string>(&processList),"Process the directories in this dir list only. this when I want to distribut my job over the nodes of ir server.")
    ("dfile",cmndOp::value<std::string>(&dumpFile),"The dump file")
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
    ("param", cmndOp::value<std::vector<std::string> >(&paramFiles), "The parameters file for indri")
    ("base-index", cmndOp::value<std::string>(&baseIndexPath)->default_value("/data/data/collections/KBA/2013/newindex"), "The base dir where all indexes are found")
    ("taggerId",cmndOp::value<std::string>(&taggerId)->default_value("lingpipe")," print anchor text");

  
  cmndOp::variables_map cmndMap;
  cmndOp::store(cmndOp::parse_command_line(argc, argv,cmndDesc), cmndMap);
  cmndOp::notify(cmndMap);  
  
  assert(logFile.size() > 0);  
  Logger::LOGGER(logFile);


  if(trainingFile.size() > 0) {
    //    storeEvaluationData(trainingFile);
    HighRecallInfo(trainingFile);
  }

  STOP_SET  = Tokenize::getStopSet(stopFile);

  std::vector<std::string> directories; 
  if(dirList.size() > 0) {
    std::ifstream inputFile(dirList.c_str());
    std::set<std::string> seen;
    for(std::string line;getline(inputFile, line);) {
      std::string dayDate = line.substr(0, line.rfind('-'));    
      if(seen.find(dayDate) == seen.end()) {
        directories.push_back(dayDate);
        seen.insert(dayDate);
      }
    }
    inputFile.close();
    std::sort(directories.begin(), directories.end(), compareString);    
  }
  
  std::vector<std::string> runDirectories; 
  //std::cout << " size " << processList.size() << std::endl;
  if(processList.size() > 0) {
    bool found = false;
    //std::string startDate = "2012-10-14-00";
    //    std::cout << " size " << processList << std::endl;
    std::ifstream inputFile(processList.c_str());
    std::set<std::string> seen;
    for(std::string line;getline(inputFile, line);) {
      //if (!found && line.compare(startDate) == 0)
      //  found = true;
      //if (!found)
      // continue;
      std::string dayDate = line.substr(0, line.rfind('-'));    
      if(seen.find(dayDate) == seen.end()) {
        runDirectories.push_back(dayDate);
	//std::cout << "Adding " << dayDate << std::endl;
        seen.insert(dayDate);
      }
    }
    inputFile.close();
    std::sort(runDirectories.begin(), runDirectories.end(), compareString);    
  }

  if(cmndMap.count("param")) {
    
    std::vector<std::string> indexDirs;
    for(std::vector<std::string>::iterator dirIt = directories.begin(); dirIt != directories.end(); ++dirIt){
      std::string indexPath = baseIndexPath + "/"+ *dirIt;
      indexDirs.push_back(indexPath);
    }
    
    std::vector<std::string> runDirs;
    for(std::vector<std::string>::iterator dirIt = runDirectories.begin(); dirIt != runDirectories.end(); ++dirIt){
      std::string indexPath = baseIndexPath + "/"+ *dirIt;
      runDirs.push_back(indexPath);
    }
    

    std::vector<kba::entity::Entity*> entityList = getEntityList(topicFile, noPositiveFile, true);
    kba::term::CorpusStat* corpusStat = new kba::term::CorpusStat();
    std::map<std::string, kba::term::TermStat*> termStatMap;
    //std::cout << " training dirs " << indexDirs.size() << " run dir "  << runDirs.size() << "\n";
    //kba::entity::Entity* entity = entityList[1];
    //std::vector<kba::entity::Entity*> temp;
    //temp.push_back(entity);
    std::map<std::string, query_t*> qMap;
    //    bootStrapIndri(qMap, indexDirs, paramFiles, temp, 20, termStatMap, corpusStat);
    makeQuery(qMap, entityList);
    //loadQueryDocuments(qMap);
    processFilterThread(qMap, runDirs, paramFiles, dumpFile, termStatMap, corpusStat);
  }

  
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
  
  /**
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
    performCCRTask(topicFile, corpusPath, dumpFile, directories, noPositiveFile, STOP_SET);
  }
  */
  
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
