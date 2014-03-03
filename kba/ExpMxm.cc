
#include "ExpMxm.hpp"


/*
 * This is what we are going to estimate topic is our hidden variable.
 */
double expmxm::probTopicGivenWord(std::string& word, std::string& topic, std::map<std::string, expmxm::TopicParam>& parameters) {
  double jntProb  = expmxm::jointProbWordTopic(word, topic, parameters);
  double wordProb = expmxm::probOfWord(word, parameters);
  return jntProb/wordProb;
}

double probWordGivenTopic(std::string word, std::string topic) {
}

double expmxm::probOfTopic(std::string topic, std::vector<std::string> words, std::map<std::string,expmxm::TopicParam>& parameters) {
  double topicExpectation = 0;
  for(std::vector<std::string>::iterator vecIt = words.begin(); vecIt != words.end(); ++vecIt) {
    std::string word = *vecIt;
    topicExpectation += expmxm::jointProbWordTopic(word, topic, parameters);
  }
  double topicProb = topicExpectation / words.size();
}

double expmxm::estimateLambda(std::string& word, std::string& topic, std::vector<std::string> words, std::map<std::string, expmxm::TopicParam>& parameters) {
  double wordExpectation = 0;
  double topicExpectation = 0;

  double freq = 0;
  for(std::vector<std::string>::iterator vecIt = words.begin(); vecIt != words.end(); ++vecIt) {
    std::string term = *vecIt;
    if(word.compare(term) == 0)
      freq += 1;
    topicExpectation += expmxm::jointProbWordTopic(word, topic, parameters);
  }
  wordExpectation = freq * expmxm::jointProbWordTopic(word, topic, parameters);   
  double lambda = wordExpectation / topicExpectation;
  return estimateLambda;
}


double expmxm::jointProbWordTopic(std::string& word, std::string& topic, st parameters) {
  double prob = parameters.varParam.at(topic).paramMap.at(word) * parameters.topicMap.at(topic);
  return prob;
}


double expmxm::probOfWord(std::string word, expmxm::Param& parameters) {
  std::map<std::string, double>& topicMap = parameters.topicMap;
  double prob = 0;
  for(std::map<std::string, double>::iterator tpcIt = topicMap.begin(); tpcIt != topicMap.end(); ++tpcIt) {
    std::string topic = tpcIt->first;
    prob += expmxm::jointProbWordTopic(word, topic, parameters);
  }
  return prob;
}

/**
 * We have to find the joint probabilty of hidden clas variable and the words.
 * Now we consider the the hidden variable of two types , one is the topic variable 
 * and the other is global variable. We consider each word is sample from one of the topic variables.
 * And each variable is sample independently.
 * http://cs.stackexchange.com/questions/10637/applying-expectation-maximization-to-coin-toss-examples
 */
void likelihoodFunction(std::vector<std::string> observations, Param& parameters) {
  double likelihood = 0;
  for(std::vector<std::string>::iterator vecIt = observations.begin(); vecIt != observations.end(); ++vecIt) {
    std::string word = *vecIt;
    std::map<std::string, double>& tpcMap = parameters.topicMap;
    fop(std::map<std::string,double>::iterator mapIt = tpcMap.begin(); mapIt !=tpcMap.end(); ++mapIt) {
      std::string& topic = mapIt->first;
      double logWordProb = log(expmxm::jointProbWordTopic(word, topic, parameters));
      logWordProb = expmxm::probTopicGivenWord(word, topic, parameters);
      likelihood += logWordProb;
    }
  }
  return likelihood;
}

void maximization() {
}

void iteration() {
}
