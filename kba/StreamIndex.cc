#include "StreamIndex.hpp"
#include "ParsedStream.hpp"
#include "stdexcept"
#include "TimeConversion.hpp"
#include <cassert>
#include <unordered_set>
#include <boost/shared_ptr.hpp>

time_t kba::StreamIndex::secondsInDay = (3600 * 24);
int16_t kba::StreamIndex::ratingAcceptance = 2;

kba::StreamIndex::StreamIndex(std::vector<std::string> dirsToProcess , std::map<TopicTermKey*, TopicTermValue*> termTopicMap, CorpusStat* corpusStat, std::set<TopicStat*> topicStat, std::set<TermStat*> termStat, kba::berkley::CorpusDb* corpusDb) : 
  _docSize(0), _numDoc(0), _dirsToProcess(dirsToProcess), _termTopicMap(termTopicMap), _corpusStat(corpusStat), _topicStatSet(topicStat), _termStatSet(termStat), _corpusDb(corpusDb) 
  {
    std::string day =  _dirsToProcess.at(0);
    day = day.substr(0, day.rfind("-"));
    _collectionTime =  kba::time::convertDateToTime(day);  
  }


int16_t kba::StreamIndex::getRating(streamcorpus::StreamItem* streamItem, std::string topic) {
  using namespace boost;
  using namespace kba::term;
  using namespace std;
  std::vector<boost::shared_ptr<EvaluationData> > ratings = StreamIndex::_corpusDb->getEvaluationData(streamItem->stream_id, topic);
  int16_t rating = 3;
  for(vector<boost::shared_ptr<EvaluationData> > ::iterator ratingIt = ratings.begin(); ratingIt != ratings.end(); ++ratingIt) {
    EvaluationData* eval = (*ratingIt).get();
    if(eval->rating < rating)
      rating = eval->rating;
  } 
  return rating >=3 ? -2 : rating;
}

void kba::StreamIndex::assertCollectionTime() {
  assert((_corpusStat->collectionTime - _collectionTime) >= kba::StreamIndex::secondsInDay);
}

void kba::StreamIndex::processStream(streamcorpus::StreamItem* streamItem ) {
  using namespace kba::term;
  std::unordered_set<std::string> dummySet;
  std::map<std::string, int16_t> topicStreamRating;

  bool judgedFlag = false;

  for(std::set<kba::term::TopicStat*>::iterator topicIt = _topicStatSet.begin(); topicIt != _topicStatSet.end(); ++topicIt) {
   
    TopicStat* topicStat = (*topicIt);

    int16_t rating = StreamIndex::getRating(streamItem, topicStat->topic);

    if(rating >= -1 && rating <= 2)
      judgedFlag = true;

    topicStreamRating.insert(std::pair<std::string, int16_t>((topicStat->topic), rating));
    if(rating >= kba::StreamIndex::ratingAcceptance) {
      topicStat->relevantSetSize += 1;
      topicStat->collectionTime = kba::StreamIndex::_collectionTime;
    }
  }

  kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createLightParsedStream(streamItem, dummySet);

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt  ) {
    TermStat* termStat = *(termIt);
    if((parsedStream->tokenSet).find(termStat->term) != (parsedStream->tokenSet).end()) 
      termStat->docFreq += 1;  
    termStat->collectionTime = StreamIndex::_collectionTime;
  }

  for(std::map<kba::term::TopicTermKey*, kba::term::TopicTermValue*>::iterator ttkIt = _termTopicMap.begin(); ttkIt != _termTopicMap.end(); ++ttkIt) {
    TopicTermKey* topicTermKey = (*ttkIt).first;
    TopicTermValue* topicTermVal = (*ttkIt).second;
    topicTermKey->collectionTime = StreamIndex::_collectionTime;
    int16_t rating = topicStreamRating.at(topicTermKey->topic);

    if((parsedStream->tokenSet).find(topicTermKey->term) != (parsedStream->tokenSet).end()) {
      if(judgedFlag)
        topicTermVal->judgedDocFreq += 1;
      if(rating >= StreamIndex::ratingAcceptance) {
        topicTermVal->relevantDocFreq += 1; 
      }
    }  
  }

  if(judgedFlag)
    _corpusStat->judgedSample += 1;
  _corpusStat->collectionTime = StreamIndex::_collectionTime;
  _corpusStat->totalDocs +=1;
  _docSize = _docSize + parsedStream->size;
  _numDoc = _numDoc + 1;
}


void kba::StreamIndex::flushToDb() {

  float currentAvgDocSize = _docSize/_numDoc;
  if (_corpusStat->averageDocSize > 0)
    currentAvgDocSize = (currentAvgDocSize + _corpusStat->averageDocSize) / 2.0; 
  _corpusStat->averageDocSize = (int)(currentAvgDocSize + 0.5); // rounded or floored depedinging  
  _corpusDb->addCorpusStat(_corpusStat);
  
  for(std::set<kba::term::TopicStat*>::iterator topicIt = _topicStatSet.begin(); topicIt != _topicStatSet.end(); ++topicIt) {
    StreamIndex::_corpusDb->addTopicStat(*topicIt);
  }

  for(std::set<TermStat*>::iterator termIt = _termStatSet.begin(); termIt != _termStatSet.end(); ++termIt  ) {
    StreamIndex::_corpusDb->addTermStat(*termIt);
  }
 
  for(std::map<kba::term::TopicTermKey*, kba::term::TopicTermValue*>::iterator ttkIt = _termTopicMap.begin(); ttkIt != _termTopicMap.end(); ++ttkIt) {
    TopicTermKey* topicTermKey = (*ttkIt).first;
    TopicTermValue* topicTermVal = (*ttkIt).second;
    StreamIndex::_corpusDb->addTopicTerm(topicTermKey, topicTermVal);
  }
} 

