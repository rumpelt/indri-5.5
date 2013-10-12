#ifndef STREAMTHREAD_HPP
#define STREAMTHREAD_HPP
#include <cstdio>
#include <string>
#include "Scorer.hpp"
#include "DumpKbaResult.hpp"
#include <boost/thread.hpp>
#include <unordered_set>

namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
    std::string _fileName; // a path can be directory to process or file to process//
    std::fstream*  _dumpStream; // some kind of buffer to dump the contents;
    kba::scorer::Scorer* _scorer;
    boost::mutex* _lockMutex; // To be used for exclusive locking    , do not fee this at the end of StreamThread because it might be used by other thread.
    std::unordered_set<std::string> _stopSet;
  public:
    void operator()(int cutOffScore); // operator over loading , potential use as thread functor


    std::string extractDirectoryName(std::string absoluteName);
    void updateScore(std::vector<kba::dump::ResultRow>& rows, std::string& id, int& score);
    void parseFile(int cutOffScore);
    StreamThread(std::string path,  std::fstream* dumpStream, kba::scorer::Scorer* scorer, boost::mutex *locMutex, std::unordered_set<std::string>& stopSet);
    StreamThread();

  };
}
#endif
