#include <cstdio>
#include <string>
#include "Scorer.hpp"

namespace kba {
  /**
   * We are creating this class to spawn as thread later on .
   */
  class StreamThread {
  private:
    std::FILE* _file; // a path can be directory to process or file to process//
    std::string  _dumpBuffer; // some kind of buffer to dump the contents;
    kba::scorer::Scorer* _scorer;
    
  public:
    void operator()(); // operator over loading , potential use as thread functor
    StreamThread(std::string path);
    StreamThread();
    ~StreamThread();
  };
}
