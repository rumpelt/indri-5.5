#ifndef STREAMINDEX_HPP
#define STREAMINDEX_HPP

#include "StatDb.hpp"
#include "TermDict.hpp"
#include "map"
#include "WikiEntity.hpp"
#include <vector>
#include "StreamUtils.hpp"
#include <map>
#include <string>
#include <vector>
#include <boost/thread.hpp>
#include "time.h"
using namespace kba::term;
namespace kba {
  /**
   * Process stream for one given day
   */
  class StreamIndex {
  private:
    unsigned int _docSize;
    unsigned int _numDoc;

    std::map<TopicTermKey, TopicTermValue> _termTopicMap;
    kba::term::CorpusStat* _corpusStat;
    std::set<TopicStat> _topicStatSet;
    std::set<TermStat*> _termStatSet;
    StatDb* _stDb;
    time_t _collectionTime;
    std::unordered_set<std::string> _stopSet;    
    std::set<std::string> _termsToFetch;
    
    void assertCollectionTime();    

    void processStream(streamcorpus::StreamItem* streamItem);
    void processStreamBasicStat(streamcorpus::StreamItem* streamItem);
    void flushToDb();

    /**
     * Not thread safe
     */
    void flushBasicStatToDb();
    void processFile(std::string fileName);

  public:
    static time_t secondsInDay;
   
    StreamIndex(std::map<TopicTermKey, TopicTermValue>& termTopicMap, CorpusStat* corpusStat, std::set<TopicStat>& topicStat, std::set<TermStat*> termStat, StatDb* stDb, std::unordered_set<std::string> stopSet, std::set<std::string> _termsToFetch);
    
    void processDir(std::vector<std::string>& dirsToProcess);
    /**
     * Needed to avoid construction and destruction of the Object
     */
    void reset();
    void setCollectionTime(time_t cTime); 
  };
}

#endif
