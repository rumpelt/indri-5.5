#ifndef PARSEDSTREAM_HPP
#define PARSEDSTREAM_HPP
#include <vector>
#include <unordered_set>

namespace kba {
  namespace stream {
    struct ParsedStream {
      std::vector<std::string> tokens; 
      std::unordered_set<std::string> tokenSet;
    };
  }
}
#endif
