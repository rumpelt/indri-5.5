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
      std::string stream_id;
      std::string directory;
      std::string rawText;
      std::vector<std::string> tokens; 
      std::unordered_set<std::string> tokenSet;
      std::unordered_map<std::string, int> tokenFreq;
      std::unordered_map<std::string, float> langModelProb; // cache to hold some prob for language modeling , to be used in KLDivergence and LanguageModeling
      std::unordered_map<std::string, float> bm25Prob; // cache to hold some prob for bm25 score
      int size;
      ParsedStream(int streamSize) : size(streamSize) {};
      ParsedStream() : size(0){};
    };
    void populateTokenFreq(kba::stream::ParsedStream* parsedStream);
    
  }
  
}
#endif
