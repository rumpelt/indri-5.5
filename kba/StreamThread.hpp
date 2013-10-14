#ifndef STREAMTHREAD_HPP
#define STREAMTHREAD_HPP
#include <cstdio>
#include <string>
#include "Scorer.hpp"
#include "DumpKbaResult.hpp"
#include "TermDict.hpp"
#include <boost/thread.hpp>
#include <unordered_set>

namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
    std::string _fileName; // a path can be directory to process or file to process//
    /**
     * Not thread safe
     */
    std::fstream*  _dumpStream; // some kind of buffer to dump the contents;
    /**
     * Not threadsafe
     */
    kba::scorer::Scorer* _scorer;
    boost::mutex* _lockMutex; // To be used for exclusive locking    , do not fee this at the end of StreamThread because it might be used by other thread.
    /**
     * Not Thread safe
     */
    std::unordered_set<std::string> _stopSet;
    /**
     * Not Thread safe
     */
    kba::term::TermBase* _termBase;

  public:

    void setTermBase(kba::term::TermBase* termBase);
    void operator()(int cutOffScore); // operator over loading , potential use as thread functor


    std::string extractDirectoryName(std::string absoluteName);

    void parseFile(int cutOffScore);
    StreamThread(std::string path,  std::fstream* dumpStream, kba::scorer::Scorer* scorer, boost::mutex *locMutex, std::unordered_set<std::string>& stopSet);
    StreamThread();

  };
}

inline void kba::StreamThread::setTermBase(kba::term::TermBase* termBase) { kba::StreamThread::_termBase = termBase;}


#endif
