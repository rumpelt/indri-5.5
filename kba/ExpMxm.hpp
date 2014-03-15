#ifndef EXPMXM_HPP
#define EXPMXM_HPP

#include "string"
#include "map"
#include "vector"

namespace expmxm {
  
  /**
   * This is conditional prob of word given topic
   */
  struct VarParam {
    std::map<std::string, double> paramMap;
  };
  
  /**
   * This is the probablity of topic
   */
  struct TopicParam {
    std::string topic;
    double topicProb; // Prob of topic, initailized and later to be etitmate and maximizied
    std::map<std::string, double> condWordProb; // Various prob of words given topic..Intialized and then later ot be estimated.
    std::map<std::string, double> condTopicProb; // probability of this topic given the word
    std::map<std::string, double> lambdaEstimate;
    std::map<std::string, double> freqMap;
  };

 
  
  /**
   * This is what we are going to estimate
   */
  double probTopicGivenWord(std::string& word, std::string& topic, std::map<std::string, TopicParam>& topicParam);
  
  double probOfTopic(std::string topic, std::map<std::string, TopicParam>& parameters);

  double jointProbWordTopic(std::string& word, std::string& topic, std::map<std::string, TopicParam>& parameters);
  
  double probOfWord(std::string& word, std::map<std::string, TopicParam>& parameters);
   
  double estimateLambda(std::string& word, std::string& topic, unsigned long docSize, std::map<std::string, TopicParam>& parameters);

  void estimateTopicCondProb(std::vector<std::string>& text, std::map<std::string, TopicParam>& parameters);

  void maximizeLambda(std::vector<std::string>& text, std::map<std::string, TopicParam>& parameters);

  void maximizeTopicProb(std::vector<std::string>& words, std::map<std::string, TopicParam>& parameters);

  double likelihoodFunction(std::vector<std::string>& observations, std::map<std::string, TopicParam>& parameters);
  void updateWordProb(std::vector<std::string>& words, std::map<std::string, TopicParam>& parameters);
  void initializeWordProb(TopicParam& topicParam, unsigned long docSize);
  void iteration(std::string& id, std::vector<std::string>& text, std::map<std::string, double>& collectionProb, std::map<std::string, double>& termProb);
  void printProb(std::vector<std::string>& text, TopicParam& topic); 
}
#endif
