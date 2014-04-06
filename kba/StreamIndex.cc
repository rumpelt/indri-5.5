#include "StreamIndex.hpp"
#include "ParsedStream.hpp"
#include "stdexcept"
#include "TimeConversion.hpp"
#include <cassert>
#include <unordered_set>
#include <boost/shared_ptr.hpp>
#include "ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"

time_t kba::StreamIndex::secondsInDay = (3600 * 24);


kba::StreamIndex::StreamIndex(std::map<TopicTermKey, TopicTermValue>& termTopicMap, CorpusStat* corpusStat, std::set<TopicStat>& topicStat, std::set<TermStat*> termStat, StatDb* stDb, std::unordered_set<std::string> stopSet,
   std::set<std::string> termsToFetch) : 
  _docSize(0), _numDoc(0),  _termTopicMap(termTopicMap), _corpusStat(corpusStat), _topicStatSet(topicStat), _termStatSet(termStat), _stDb(stDb), _stopSet(stopSet), _termsToFetch(termsToFetch) 
  {
  }




void kba::StreamIndex::assertCollectionTime() {
  assert((_corpusStat->collectionTime - _collectionTime) >= kba::StreamIndex::secondsInDay);
}


void kba::StreamIndex::processStream(streamcorpus::StreamItem* streamItem ) {
  using namespace kba::term;

  std::map<std::string, int16_t> topicStreamRating;

  bool judgedFlag = false;
  kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createMinimalParsedStream(streamItem, _stopSet, _termsToFetch);

  for(std::set<kba::term::TopicStat>::iterator topicIt = _topicStatSet.begin(); topicIt != _topicStatSet.end(); ++topicIt) {
   
    TopicStat topicStat = (*topicIt);

    int16_t rating = -1;
    //StreamIndex::getRating(streamItem, topicStat.topic);

    if(rating >= -1 && rating <= 2)
      judgedFlag = true;

    topicStreamRating.insert(std::pair<std::string, int16_t>((topicStat.topic), rating));
    /**
    if(rating >= kba::StreamIndex::ratingAcceptance) {
      topicStat.relevantSetSize += 1;
      topicStat.collectionTime = kba::StreamIndex::_collectionTime;
    }
    */
  }

  

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt  ) {
    TermStat* termStat = *(termIt);
    try {
      int freq = parsedStream->tokenFreq.at(termStat->term);
      termStat->docFreq += 1;  
      termStat->collFreq += freq;
      termStat->collectionTime = StreamIndex::_collectionTime;
    }
    catch(std::out_of_range& oor) {
 
    }
  }

 if(judgedFlag)
    _corpusStat->judgedSample += 1;
  _corpusStat->collectionTime = StreamIndex::_collectionTime;
  _corpusStat->totalDocs +=1;
  _docSize = _docSize + parsedStream->size;
  _numDoc = _numDoc + 1;

  delete parsedStream;
}

void kba::StreamIndex::processStreamBasicStat(streamcorpus::StreamItem* streamItem) {
  using namespace kba::term;


  kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createMinimalParsedStream(streamItem, _stopSet, _termsToFetch);
  

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt  ) {
    TermStat* termStat = *(termIt);
    try {
      int freq = parsedStream->tokenFreq.at(termStat->term);
      termStat->docFreq += 1;  
      termStat->collFreq += freq;
    }
    catch(std::out_of_range& oor) {
 
    }
  }
  
  _corpusStat->totalDocs +=1;
  _docSize = _docSize + parsedStream->size;
  _numDoc = _numDoc + 1;

  delete parsedStream;
}

/**
 * No thread safe
 */
void kba::StreamIndex::flushBasicStatToDb() {
 
  float currentAvgDocSize = _docSize/_numDoc;
  int crpAvgDoc = _corpusStat->averageDocSize;
  if ( crpAvgDoc > 0)
    currentAvgDocSize = (currentAvgDocSize + crpAvgDoc) / 2.0; 
  _corpusStat->averageDocSize = (int)(currentAvgDocSize + 0.5); // rounded or floored depedinging    
  _corpusStat->collectionTime = StreamIndex::_collectionTime;
  StreamIndex::_stDb->wrtCrpSt(_corpusStat);

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt) {
    TermStat* termSt = *termIt;
    termSt->collectionTime = StreamIndex::_collectionTime;
    StreamIndex::_stDb->wrtTrmSt(*termIt);
  }
} 

void kba::StreamIndex::flushToDb() {
 
  float currentAvgDocSize = _docSize/_numDoc;
  if (_corpusStat->averageDocSize > 0)
    currentAvgDocSize = (currentAvgDocSize + _corpusStat->averageDocSize) / 2.0; 
  _corpusStat->averageDocSize = (int)(currentAvgDocSize + 0.5); // rounded or floored depedinging    
  //  _corpusDb->addCorpusStat(_corpusStat);
  //std::cout << "Corpus Stat "<< _corpusStat->collectionTime << " total doc " << _corpusStat->totalDocs << " Average doc " << _corpusStat->averageDocSize;

  StreamIndex::_stDb->wrtCrpSt(_corpusStat);
  for(std::set<kba::term::TopicStat>::iterator topicIt = _topicStatSet.begin(); topicIt != _topicStatSet.end(); ++topicIt) {
    //StreamIndex::_corpusDb->addTopicStat(*topicIt);
    //   StreamIndex::_stDb->wrtCrpSt(*topicIt);
  }

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt  ) {
    //StreamIndex::_corpusDb->addTermStat(*termIt);
    StreamIndex::_stDb->wrtTrmSt(*termIt);
  }
 
} 


void kba::StreamIndex::processFile(std::string fileName) {
  std::string fileExtension = fileName.substr(fileName.size() - 3);
  if(fileExtension.compare(".xz")) {
    return; // do nothing
  }      
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(fileName);
  streamcorpus::StreamItem* streamItem = 0;
  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    StreamIndex::processStreamBasicStat(streamItem);
  }
  delete tdextractor;
}

void kba::StreamIndex::processDir(std::vector<std::string>& dirsToProcess) {

 for(std::vector<std::string>::iterator dirIt= dirsToProcess.begin(); dirIt != dirsToProcess.end(); ++dirIt) {
    std::string pathToProcess = *dirIt;
    indri::file::FileTreeIterator files(pathToProcess);
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      StreamIndex::processFile(*files);
    }
  }
  StreamIndex::flushBasicStatToDb();
}

void kba::StreamIndex::reset() {
  _docSize = 0;
  _numDoc = 0;
  _collectionTime = -1;
}

void kba::StreamIndex::setCollectionTime(time_t cTime) {
  _collectionTime = cTime;
  assert(_collectionTime > 0);
}
