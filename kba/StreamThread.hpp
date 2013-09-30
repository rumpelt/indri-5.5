#ifndef STREAMTHREAD_HPP
#define STREAMTHREAD_HPP
#include <cstdio>
#include <string>
#include "Scorer.hpp"
#include <boost/thread.hpp>
namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
    std::string _fileName; // a path can be directory to process or file to process//
    std::fstream*  _dumpStream; // some kind of buffer to dump the contents;
    kba::scorer::Scorer* _scorer;
    boost::mutex* _lockMutex; // To be used for exclusive locking     
  public:
    void operator()(); // operator over loading , potential use as thread functor
    std::string extractDirectoryName(std::string absoluteName);
    void parseFile();
    StreamThread(std::string path,  std::fstream* dumpStream, kba::scorer::Scorer* scorer, boost::mutex *locMutex);
    StreamThread();

  };
}
#endif
