#ifndef PARSEDSTREAM_HPP
#define PARSEDSTREAM_HPP

#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>


namespace kba {
  namespace stream {
    struct ParsedStream {
      std::vector<std::string> tokens; 
      std::unordered_set<std::string> tokenSet;
      std::unordered_map<std::string, int> tokenFreq;
      int size;
      ParsedStream() : size(-1) {};
    };
    void populateTokenFreq(kba::stream::ParsedStream* parsedStream);
    
  }
  
}
#endif
