#ifndef PARSEDSTREAM_HPP
#define PARSEDSTREAM_HPP

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include "TermDict.hpp"

namespace kba {
  namespace stream {
    struct ParsedStream {
      std::vector<std::string> tokens; 
      std::unordered_set<std::string> tokenSet;
      std::unordered_map<std::string, int> tokenFreq;
    };
    void populateTokenFreq(kba::stream::ParsedStream* parsedStream);
    void updateTermBase(kba::stream::ParsedStream* parsedStream, kba::term::TermBase* termBase, std::set<std::string> termSet);
  }
  
}
#endif
