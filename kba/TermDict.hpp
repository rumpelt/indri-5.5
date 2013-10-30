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
#include "time.h"
#include "boost/shared_ptr.hpp"

namespace kba {
  namespace term {
    
    /**
     * You will have to some marshaling   if you have to store in bdb
     */
    struct TopicTermKey {
      time_t collectionTime;
      std::string term;  
      std::string topic;
      TopicTermKey() : collectionTime(-1){};
      bool operator < (const TopicTermKey& rhs) {
        if(!term.compare(rhs.term) && !topic.compare(rhs.topic))
          return false;
        return true; 
      } 
    };
         
    struct TopicTermValue {
      unsigned int judgedDocFreq; //  Number of judged doc containing term,  n_i term in the Robertson/Sprck Jones
      unsigned int relevantDocFreq; // Number of the relevant documents containing term, r_i term in Robertosn/sprck Jones weight
      TopicTermValue() : judgedDocFreq(0), relevantDocFreq(0) {};
    };
   
    struct TopicStat {
      time_t collectionTime;    // This is Primary key along wit the topic Name as following
      std::string topic; //  This is primary key along with above, Topic for which relevantSetSize below is provided, duplcate records should be allowed    
      int relevantSetSize;  // Relevant set Size , Term R in the Robertson/Sprck Jone weightin
      TopicStat() : collectionTime(-1), relevantSetSize(0) {};
      
      bool operator<(const TopicStat& rhs) {
	
        if(topic.compare(rhs.topic) == 0)
          return false;
        if(topic.compare(rhs.topic) < 0)
          return true;
        else
          return false;
      }
    };

    
    struct CorpusStat {
      time_t collectionTime;   // This should be primary key. 
      long totalDocs;
      long judgedSample; // The N value
      int averageDocSize;  // Rounded to integer value;
      CorpusStat() : collectionTime(0), totalDocs(0), judgedSample(0), averageDocSize(0) {}
    };

    struct TermStat {
      time_t collectionTime; // this is the primary key along with the term
      std::string term;
      long docFreq; // For calculating IDF
      long int collFreq; //Total frequency of the term in the corpus. this is greater than Id. This to calculate the colelection Term Frequency  
      bool operator() (TermStat* lhs, TermStat* const rhs)  {
        std::cout << "Calling functor\n";
        if((lhs->term).compare(rhs->term) == 0)
          return false;
        if((lhs->term).compare(rhs->term) < 0)
          return true;
        else
          return false;
      }
      bool operator() (TermStat lhs, TermStat const rhs)  {
        std::cout << "Calling functor\n";
        if((lhs.term).compare(rhs.term) == 0)
          return false;
        if((lhs.term).compare(rhs.term) < 0)
          return true;
        else
          return false;
      }
      TermStat() : collectionTime(-1), docFreq(0),  collFreq(0) {}
    };
     
    struct TermStatCmp  {
      bool operator() (TermStat* lhs, TermStat* const rhs)  {
        std::cout << "Calling functor\n";
        if((lhs->term).compare(rhs->term) == 0)
          return false;
        if((lhs->term).compare(rhs->term) < 0)
          return true;
        else
          return false;
      } 
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


    /**
     * To be used to save the evaluation data in berkley db.
     * Duplicate data should be allowed
     */
    struct EvaluationData {
      time_t timeStamp; // time stamp part of the stream-id
      std::string docId; // doc id part of the stream-id. Combinatin of the above two form the stream-id      
      std::string topic; 
      int16_t rating;  
      int cleanVisibleSize;     
      std::string directory; 
      EvaluationData() : rating(-2), cleanVisibleSize(0) {}
    };

    float& LOG_OF_2();   

    /**
     * This is not thread safe and so u must synchronize this
     */
    void updateTermBase(kba::stream::ParsedStream* parsedStream, kba::term::TermBase* termBase);
    
    void populateVocabulary(std::vector<kba::entity::Entity*> entityList, kba::term::TermBase* termBase);

    std::set<TopicStat*> crtTopicStatSet(std::vector<kba::entity::Entity*> entitySet);    
    std::set<TermStat*> crtTermStatSet(std::vector<kba::entity::Entity*> entitySet, std::unordered_set<std::string> stopSet);    
    std::map<TopicTermKey*, TopicTermValue*>  crtTopicTermMap(std::vector<kba::entity::Entity*> entitySet, std::unordered_set<std::string> stopSet);    
  }
}

#endif
