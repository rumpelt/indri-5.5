#ifndef STREAMINDEX_HPP
#define STREAMINDEX_HPP

#include "BerkleyDBEnv.hpp"
#include "TermDict.hpp"
#include "map"
#include "WikiEntity.hpp"
#include <vector>
#include "StreamUtils.hpp"
#include <map>
#include <string>
#include <vector>

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
    std::vector<std::string> _dirsToProcess;

    std::map<TopicTermKey*, TopicTermValue*> _termTopicMap;
    kba::term::CorpusStat* _corpusStat;
    std::set<TopicStat*> _topicStatSet;
    std::set<TermStat*> _termStatSet;
    kba::berkley::CorpusDb* _corpusDb;

    time_t _collectionTime;
    

    void assertCollectionTime();    
    void processStream(streamcorpus::StreamItem* streamItem);
    void flushToDb();
  public:
    static time_t secondsInDay;
    static int16_t ratingAcceptance;

    StreamIndex(std::vector<std::string> dirsToProcess , std::map<TopicTermKey*, TopicTermValue*> termTopicMap, CorpusStat* corpusStat, std::set<TopicStat*> topicStat, std::set<TermStat*> termStat, kba::berkley::CorpusDb* corpusDb);
           
  };
}

#endif
