#include "StreamThread.hpp"
#include "StreamUtils.hpp"
#include "DumpKbaResult.hpp"
#include "LanguageModel.hpp"
#include "BM25Scorer.hpp"
#include "LanguageModelExt.hpp"
#include "BM25ScorerExt.hpp"
#include "KLDivergence.hpp"
#include <iostream>
#include <map>
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
using namespace kba::term;

kba::StreamThread::StreamThread(std::vector<std::string> dirsToProcess, std::fstream* dumpStream, std::vector<kba::entity::Entity*> entities, std::unordered_set<std::string> stopSet, std::string date, int cutoffScore) :
  _dirsToProcess(dirsToProcess), _dumpStream(dumpStream), _entities(entities), _stopSet(stopSet) ,  _date(date), _cutoffScore(cutoffScore) {
  _crpStat= 0;

}

void kba::StreamThread::setCorpusStat(CorpusStat* crpStat) {
  _crpStat = crpStat;
}

void kba::StreamThread::setTermStat(std::map<std::string, TermStat*> termStMap) {
  _trmStatMap = termStMap;
}

void kba::StreamThread::setTopicTerm(std::set<TopicTerm*> tpcTerm) {
  _tpcTrm = tpcTerm;
}

/**
 * can return empty string
 */
std::string kba::StreamThread::extractDirectoryName(std::string absoluteName) {
  size_t lastSlash = absoluteName.find_last_of('/');
  std::string dirName;
  if(lastSlash != std::string::npos) {
    size_t scndLastSlash = absoluteName.find_last_of('/', lastSlash - 1);
    if (scndLastSlash != std::string::npos) {
      dirName = absoluteName.substr(scndLastSlash + 1 , lastSlash - scndLastSlash -1);
    }
  }
  return dirName;
}

void kba::StreamThread::flushStatDb() {
  _statDb->wrtCrpSt(_crpStat);
  for(std::map<std::string, kba::term::TermStat*>::iterator termIt = _trmStatMap.begin(); termIt != _trmStatMap.end(); ++termIt) {
    kba::term::TermStat* trmSt  = termIt->second;
    _statDb->wrtTrmSt(trmSt);
  }
}

void kba::StreamThread::updateCorpusStat(CorpusStat* crpStat, long numDocs, size_t docSize) {
  std::unique_lock<std::mutex> lk(StreamThread::mut);
  crpStat->totalDocs += numDocs;
  _crpStat->collectionSize += docSize;
  crpStat->collectionTime = _timeStamp;
  double avgDocSize =  ((double)(_crpStat->collectionSize)) / (crpStat->totalDocs);
  avgDocSize += 0.5;
  _crpStat->averageDocSize = (int)(avgDocSize); // rounded or floored depedinging    
}


void kba::StreamThread::parseFile(int cutOffScore, std::string fileName, std::string dirName, std::unordered_set<std::string> docIds, bool firstPass) {
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(fileName);
  streamcorpus::StreamItem* streamItem = 0;
  std::vector<kba::stream::ParsedStream*> streams;
  std::vector<kba::TempResult> rsLst;
  long numDocs = 0;
  size_t docSize = 0;
  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    //  std::cout << "first Pass " << firstPass << "\n";  
    std::string docId  = (streamItem->stream_id).substr((streamItem->stream_id).find("-")+1);
    if ( (streamItem->body).clean_visible.size() <= 0)
      continue;
    kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createMinimalParsedStream(streamItem,_stopSet, _termSet);
    ++numDocs;
    docSize = docSize + parsedStream->size;
    
    for(std::vector<kba::entity::Entity*>::iterator entIt = _entities.begin(); entIt != _entities.end(); entIt++) {
        
      kba::entity::Entity* entity = *entIt;
      bool found = false;

      for(std::vector<std::string>::iterator tokIt=(entity->labelTokens).begin(); tokIt != (entity->labelTokens).end(); ++tokIt) {
        if((parsedStream->tokenFreq).find(*tokIt) != (parsedStream->tokenFreq).end()) {
          found = true;
          break;
	}
      }

      if(found && !firstPass) {
        for(std::vector<kba::scorer::Scorer*>::iterator scIt = _scorers.begin(); scIt != _scorers.end(); ++scIt) {
	  kba::scorer::Scorer* scorer = *scIt;
          int score = (int) (scorer->score(parsedStream, entity, 1000)); // first check we have implemented the parsedStreamMethod or not 

          if (score > -99999) {
	    kba::TempResult tmp;
            tmp.modelName = scorer->getModelName(); tmp.topicName = entity->wikiURL; tmp.stream_id = streamItem->stream_id; tmp.score = score; tmp.dirName = dirName;
            rsLst.push_back(tmp);

	  }
        }
      }
    }
    //    updateTermStat(_trmStatMap, parsedStream);  
    (parsedStream->bm25Prob).clear();
    (parsedStream->langModelProb).clear();
    streams.push_back(parsedStream);
    //delete parsedStream; 
  }

  StreamThread::addResult(rsLst);
  StreamThread::updateTermStat(_trmStatMap, streams);
  for(std::vector<kba::stream::ParsedStream*>::iterator psIt = streams.begin(); psIt != streams.end(); ++psIt)
    delete *psIt;

  updateCorpusStat(_crpStat, numDocs, docSize);
  delete tdextractor;
}

void kba::StreamThread::setTermSet(std::set<std::string> termSet) {
  _termSet = termSet;
}

void kba::StreamThread::setStatDb(StatDb* statDb) {
  StreamThread::_statDb = statDb;
}

void kba::StreamThread::createResultPool(std::string modelName, int poolSz, bool biggerIsBetter, int initValue) {
  std::map<std::string, ResultPool*>& resMap = collMap[modelName];

  for(std::vector<kba::entity::Entity*>::iterator entIt = _entities.begin(); entIt != _entities.end(); ++entIt) {
    kba::entity::Entity* entity = *entIt;    
    ResultPool* rp = new ResultPool(entity->wikiURL, poolSz, initValue, biggerIsBetter);
    resMap.insert(std::pair<std::string, ResultPool* >(entity->wikiURL, rp));
  }
}
    
void kba::StreamThread::addResult(std::string modelName, std::string entityId, std::string streamId, int score, std::string directory) {
  std::unique_lock<std::mutex> lk(StreamThread::mut);
  std::map<std::string, ResultPool*>& resMap = collMap[modelName];
  ResultPool* rp = resMap.at(entityId);
  rp->addResult(streamId, directory, score);
}

void kba::StreamThread::addResult(std::vector<kba::TempResult>& rsLst) {
  std::unique_lock<std::mutex> lk(StreamThread::mut);
  for (std::vector<kba::TempResult>::iterator rsIt = rsLst.begin(); rsIt != rsLst.end(); ++rsIt) {
    kba::TempResult rs = *rsIt;
    std::map<std::string, ResultPool*>& resMap = StreamThread::collMap[rs.modelName];
    ResultPool* rp = resMap.at(rs.topicName);
    rp->addResult(rs.stream_id, rs.dirName, rs.score);
  }
}

    
void kba::StreamThread::freeResultPool() {
  for(std::vector<kba::scorer::Scorer*>::iterator scIt = _scorers.begin(); scIt != _scorers.end(); ++scIt) {
    std::string modelName = (*scIt)->getModelName();
    std::map<std::string, ResultPool*> & resMap = collMap[modelName];
    for(std::vector<kba::entity::Entity*>::iterator entIt = _entities.begin(); entIt != _entities.end(); ++entIt) {
      ResultPool* rp = resMap.at((*entIt)->wikiURL);
      delete rp;
    }
  }
}

void kba::StreamThread::publishResult() {
  for(std::vector<kba::scorer::Scorer*>::iterator scIt = _scorers.begin(); scIt != _scorers.end(); ++scIt) {
    std::string modelName = (*scIt)->getModelName();
    std::map<std::string, ResultPool*> & resMap = collMap[modelName];
    for(std::vector<kba::entity::Entity*>::iterator entIt = _entities.begin(); entIt != _entities.end(); ++entIt) {
      kba::entity::Entity* entity = *entIt;      
      ResultPool* rp = resMap.at(entity->wikiURL);
      std::vector<ResultStruct*> resultSet = rp->getResults();
      for(std::vector<ResultStruct*>::iterator rsIt = resultSet.begin(); rsIt != resultSet.end(); ++rsIt) {
        if(((*rsIt)->id).size() > 0) // Chance are very less to be false
          *_dumpStream << (*rsIt)->id << " " << entity->wikiURL << " "<< (*rsIt)->score << " " << (*rsIt)->dayDt << " "  << modelName   << "\n";
      }
    }
  }
  (*_dumpStream).flush();
}   






void kba::StreamThread::spawnParserNScorers(bool firstPass) {
  // create the scorers here
  using namespace kba::scorer;
  //  std::cout << " date " << _date << "\n";
  _timeStamp = kba::time::convertDateToTime(_date);
  //time_t stime = _timeStamp - 7200;
  //time_t etime = _timeStamp + ((24 * 3600) - 1);
  std::unordered_set<std::string> docIds; // = _statDb->getDocIds(stime, etime, false);
  //  std::cout << " Processing "<<_date <<" Doc Ids " << docIds.size() << "\n";

  int numThreads = std::thread::hardware_concurrency();
  std::ifstream trdF("../help/cputhread");
  if (trdF.is_open()) {
    trdF >> numThreads;
    trdF.close();
  }
  else if(numThreads <= 0)
    numThreads = 16;
  //  std::cout << "Num Threads " << numThreads << "\n";
  std::vector<std::thread> threads;

  int poolSz = 100;
  //  std::cout << "first Pass " << firstPass << "\n"; 
  if(!firstPass) {
    
   BM25ScorerExt*  bmExt = new BM25ScorerExt(_entities, _crpStat, _trmStatMap, 10); 
    _scorers.push_back(bmExt);
   createResultPool(bmExt->getModelName(), poolSz, true, 0);
   
   BM25Scorer* bm = new BM25Scorer(_entities, _crpStat, _trmStatMap,2.0);
   _scorers.push_back(bm);
   createResultPool(bm->getModelName(), poolSz, true, 0);
  
   LanguageModel*  lm = new LanguageModel(_entities, _trmStatMap, _crpStat, 900.0);
   _scorers.push_back(lm);
   createResultPool(lm->getModelName(), poolSz, true, -10000);

   LanguageModelExt* lmExt = new LanguageModelExt(_entities, _trmStatMap, _crpStat, 500.0);
   _scorers.push_back(lmExt);
   createResultPool(lmExt->getModelName(), poolSz, true, -10000);
   
   KLDivergence* kl = new KLDivergence(_entities, _crpStat, _trmStatMap);
   _scorers.push_back(kl);
   createResultPool(kl->getModelName(), poolSz, true, -10000);
   
  }
  
  for(std::vector<std::string>::iterator dirIt = _dirsToProcess.begin(); dirIt != _dirsToProcess.end(); ++dirIt) {
    indri::file::FileTreeIterator files(*dirIt);
    std::string dirName = (*dirIt).substr((*dirIt).rfind("/")+1);
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      std::string fName = *files;
      if(threads.size() >= numThreads) {
	std::for_each(threads.begin(), threads.end(),std::mem_fn(&std::thread::join));
        threads.clear();
      }
      // parseFile(_cutoffScore, fName, dirName, docIds, firstPass);
      threads.push_back(std::thread(&StreamThread::parseFile, this, _cutoffScore, fName, dirName, docIds, firstPass));
    }
    
  }
  
  if(threads.size() >= 0) {
    std::for_each(threads.begin(), threads.end(),std::mem_fn(&std::thread::join));
    threads.clear();
  }

  StreamThread::flushStatDb();
  if(!firstPass) {
    StreamThread::publishResult();
    StreamThread::freeResultPool();
  }
  for(std::vector<kba::scorer::Scorer*>::iterator scIt = _scorers.begin(); scIt != _scorers.end(); ++scIt) {
    delete *scIt;
  }
}




