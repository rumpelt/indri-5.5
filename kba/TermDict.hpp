#ifndef TERMDICT_HPP
#define TERMDICT_HPP

#include  <map>
#include <string>
#include <cmath>

#ifndef LOG2
#define LOG2 LOG_OF_2()
#endif


namespace kba {
  namespace term {
    struct TermBase { 
      std::map<std::string, unsigned int> termDocFreq;
      unsigned long totalDocLength;
      unsigned long  totalDocs;
      float avgDocLength; // update this also at the end of udpating the dictionary
      std::map<std::string, float> logIDF;      // U must update the logIDF when u update the termDocFreq
      TermBase() : totalDocLength(0), totalDocs(0), avgDocLength(0.0){}
    };
    
    float& LOG_OF_2();   
    
  }
}

#endif
