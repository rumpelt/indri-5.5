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
    
    struct TopicTermKey {
      unsigned char* term;
      unsigned char* topic;
      long collectionTime;

    };
         
    struct TopicTermValue {

      unsigned int docFreq; // For calculating IDF, Not dependent on the topic
      unsigned int judgedDocFreq; //  Number of judged doc containing term,  n_i term in the Robertson/Sprck Jones
      unsigned int relevantDocFreq; // Number of the relevant documents containing term, r_i term in Robertosn/sprck Jones weight
      long judgedSample; // size of the judged sample for the topic/query, term N in the Roberson/Sprck Jones wieghting
      long relevantSetSize;  // Relevant set Size , Term R in the Robertson/Sp rck Jone weightin
    };

    struct CorpusStatKey {
      long collectionTime;
    };
    
    struct CorpusStatValue {
      long totalDocs;
      int averageDoSize;  // Rounded to integer value;
    };

    struct DocStatValue {
    };
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
