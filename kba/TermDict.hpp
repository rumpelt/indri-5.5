#ifndef TERMDICT_HPP
#define TERMDICT_HPP

#include  <map>
#include <string>
#include <cmath>
#include <iostream>

#ifndef LOG2
#define LOG2 LOG_OF_2()
#endif

#include "ParsedStream.hpp"
#include "WikiEntity.hpp"

namespace kba {
  namespace term {
    struct TermBase { 
      std::set<std::string> vocabulary;
      std::map<std::string, unsigned int> termDocFreq;
      unsigned long totalDocLength;
      unsigned long  totalDocs;
      float avgDocLength; // update this also at the end of udpating the dictionary
      std::map<std::string, float> logIDF;      // U must update the logIDF when u update the termDocFreq
      TermBase() : totalDocLength(0), totalDocs(0), avgDocLength(0.0){};
      TermBase(std::vector<kba::entity::Entity*> entityList);
    };
    
    float& LOG_OF_2();   

    /**
     * This is not thread safe and so u must synchronize this
     */
    void updateTermBase(kba::stream::ParsedStream* parsedStream, kba::term::TermBase* termBase);
    
    void populateVocabulary(std::vector<kba::entity::Entity*> entityList, kba::term::TermBase* termBase);
  }
}

#endif
