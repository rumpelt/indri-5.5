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
    std::set<kba::term::TermStat*> _trmStat;
    std::set<kba::term::TopicTerm*> _tpcTrm;
    std::set<std::string> _termSet;

    time_t _timeStamp;
    //    std::string _baseDir;
    int _cutoffScore;
    std::vector<kba::scorer::Scorer*> _scorers;
    kba::thrift::ThriftDocumentExtractor* _tdextractor;
    
  public:
    void operator()(int cutOffScore); // operator over loading , potential use as thread functor

    void spawnParserNScorers(bool firstPass);
    std::string extractDirectoryName(std::string absoluteName);
    void setTermStat(std::set<kba::term::TermStat*> termStatSet);
    void setCorpusStat(kba::term::CorpusStat*);
    void setTopicTerm(std::set<kba::term::TopicTerm*> tpcTrm);
    void setTermSet(std::set<std::string> termSet);
  
    void updateCorpusStat(kba::term::CorpusStat*, long numDocs, size_t docSize);
    void updateTermStat(std::set<kba::term::TermStat*> statSet, kba::stream::ParsedStream* stream);
    void parseFile(int cutOffScore, std::string fileName, std::string dirName, bool firstPass);
    StreamThread(std::vector<std::string> dirsToProcess,  std::fstream* dumpStream, std::vector<kba::entity::Entity*> entities, std::unordered_set<std::string> stopSet, int cuttoffScore=650);
    StreamThread();

  };
}

#endif
