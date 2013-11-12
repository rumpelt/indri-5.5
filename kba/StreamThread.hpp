#ifndef STREAMTHREAD_HPP
#define STREAMTHREAD_HPP
#include <cstdio>
#include <string>
#include "Scorer.hpp"
#include "DumpKbaResult.hpp"
#include "TimeConversion.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "TermDict.hpp"
#include <boost/thread.hpp>
#include <unordered_set>
#include "StatDb.hpp"

namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
   std::vector<std::string> _dirsToProcess;
    /**
     * Not thread safe
     */
    std::fstream*  _dumpStream; // some kind of buffer to dump the contents;
    /**
     * Not threadsafe
     */
    std::vector<kba::entity::Entity*> _entities;
    
    /**
     * Not Thread safe
     */
    std::unordered_set<std::string> _stopSet;
    kba::term::CorpusStat* _crpStat;
    std::map<std::string, kba::term::TermStat*> _trmStatMap;
    std::set<kba::term::TopicTerm*> _tpcTrm;
    std::set<std::string> _termSet;
    StatDb* _statDb;
    time_t _timeStamp;
    std::string _date;
    int _cutoffScore;
    std::vector<kba::scorer::Scorer*> _scorers;
    kba::thrift::ThriftDocumentExtractor* _tdextractor;
    
  public:
    void operator()(int cutOffScore); // operator over loading , potential use as thread functor

    void spawnParserNScorers(bool firstPass);
    std::string extractDirectoryName(std::string absoluteName);
    void setTermStat(std::map<std::string, kba::term::TermStat*> termStatMap);
    void setCorpusStat(kba::term::CorpusStat*);
    void setTopicTerm(std::set<kba::term::TopicTerm*> tpcTrm);
    void setTermSet(std::set<std::string> termSet);
    void setStatDb(StatDb* statDb);
 
    void updateCorpusStat(kba::term::CorpusStat*, long numDocs, size_t docSize);
    void updateTermStat(std::map<std::string, kba::term::TermStat*> statMap, kba::stream::ParsedStream* stream);
    void flushStatDb();
    void parseFile(int cutOffScore, std::string fileName, std::string dirName, std::unordered_set<std::string> docIds, bool firstPass);
    StreamThread(std::vector<std::string> dirsToProcess,  std::fstream* dumpStream, std::vector<kba::entity::Entity*> entities, std::unordered_set<std::string> stopSet, std::string _date, int cuttoffScore=650);
    StreamThread();

  };
}

inline void kba::StreamThread::updateTermStat(std::map<std::string, TermStat*> termStatMap, kba::stream::ParsedStream* stream) {

  for(std::map<std::string, kba::term::TermStat*>::iterator termIt = termStatMap.begin(); termIt != termStatMap.end(); ++termIt) {
    kba::term::TermStat* trmSt  = termIt->second;
    trmSt->collectionTime = _timeStamp;
    try {
      int freq = stream->tokenFreq.at(trmSt->term);
      trmSt->docFreq += 1;
      trmSt->collFreq += freq;
    } catch (std::out_of_range& oor) {
    }
  }
}

#endif
