#ifndef STREAMTHREAD_HPP
#define STREAMTHREAD_HPP
#include <cstdio>
#include <string>
#include "Scorer.hpp"

namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
    std::string _fileName; // a path can be directory to process or file to process//
    std::string  _dumpFileName; // some kind of buffer to dump the contents;
    kba::scorer::Scorer* _scorer;
    
  public:
    void operator()(); // operator over loading , potential use as thread functor
    std::string extractDirectoryName(std::string absoluteName);
    void parseFile();
    StreamThread(std::string path,  std::string dumpFileName, kba::scorer::Scorer* scorer);
    StreamThread();

  };
}
#endif
