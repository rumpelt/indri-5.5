#include "ExpMxm.hpp"
#include "MathRoutines.hpp"
#include "math.h"
#include "set"
#include <iostream>
/*
 * This is what we are going to estimate topic is our hidden variable.
 */
double expmxm::probTopicGivenWord(std::string& word, std::string& topic, std::map<std::string, expmxm::TopicParam>& parameters) {
  double jntProb  = expmxm::jointProbWordTopic(word, topic, parameters);
  
  double wordProb = expmxm::probOfWord(word, parameters);
  //  std::cout << std::endl;
  //std::cout << "Prob of topcic/Word " << word << " Joint pob " <<  jntProb << " word prob " << wordProb << " cond prob " << parameters[topic].condWordProb[word] << std::endl;
  return jntProb/wordProb;
}



double expmxm::estimateLambda(std::string& word, std::string& topic, unsigned long docSize, std::map<std::string, expmxm::TopicParam>& parameters) {
  double wordExpectation = 0;
  double topicExpectation = 0 ; //docSize * parameters[topic].topicProb;
  expmxm::TopicParam& topicParam = parameters[topic];

  for(std::map<std::string,double>::iterator mapIt = topicParam.freqMap.begin(); mapIt != topicParam.freqMap.end(); ++mapIt) {
    std::string term = mapIt->first;
    double freq = mapIt->second;
  
    topicExpectation += freq * topicParam.condTopicProb[term];
    //    std::cout << "Freq: " <<freq << " cond prob " << topicParam.condTopicProb[term] << std::endl;
  }

  
  double freq = parameters[topic].freqMap[word];

  wordExpectation = freq * parameters[topic].condTopicProb[word];   
  //  std::cout << "Word Exp: " << wordExpectation << " topic: " << topicExpectation << " estimate: " << wordExpectation/topicExpectation << std::endl; 
  double lambda = wordExpectation / topicExpectation;
  return lambda;
}

void expmxm::estimateTopicCondProb(std::vector<std::string>& text, std::map<std::string, expmxm::TopicParam>& parameters) {
  std::set<std::string> seenWords;
  for(std::vector<std::string>::iterator termIt = text.begin(); termIt != text.end(); ++termIt) {
    std::string term = *termIt;
    if(seenWords.find(term) != seenWords.end())
      continue;
    for(std::map<std::string, expmxm::TopicParam>::iterator mapIt = parameters.begin(); mapIt != parameters.end(); ++mapIt) {
      expmxm::TopicParam& topic = mapIt->second;
      double prob = expmxm::probTopicGivenWord(term, topic.topic, parameters);
      //      std::cout << "Setting topic " << term  << " " << prob ;
      topic.condTopicProb.insert(std::pair<std::string, double>(term, prob));
      //      std::cout << topic.condTopicProb[term];
    }
    //    std::cout << std::endl;
    seenWords.insert(term);
  }
}


void expmxm::maximizeLambda(std::vector<std::string>& text, std::map<std::string, expmxm::TopicParam>& parameters) {

  std::set<std::string> seenWords;
  for(std::vector<std::string>::iterator termIt = text.begin(); termIt != text.end(); ++termIt) {
    std::string term = *termIt;
    if(seenWords.find(term) != seenWords.end())
      continue;

    for(std::map<std::string, expmxm::TopicParam>::iterator mapIt = parameters.begin(); mapIt != parameters.end(); ++mapIt) {
      std::string topic = mapIt->first;
      expmxm::TopicParam& topicParam = mapIt->second;
      double lambda = expmxm::estimateLambda(term, topic, text.size(), parameters);
      topicParam.lambdaEstimate[term] = lambda;
    }
    seenWords.insert(term);
  }
}

void expmxm::maximizeTopicProb(std::vector<std::string>& words, std::map<std::string, TopicParam>& parameters) {
  for(std::map<std::string, expmxm::TopicParam>::iterator mapIt = parameters.begin(); mapIt != parameters.end(); ++mapIt) {
    std::string topic = mapIt->first;
    expmxm::TopicParam& topicParam = mapIt->second;
    double topicExpectation;// = words.size() * topicParam.topicProb;
    
    for(std::vector<std::string>::iterator wordIt = words.begin(); wordIt != words.end(); ++wordIt) {
      std::string word = *wordIt;
      topicExpectation += topicParam.condTopicProb[word];
    }
    
    double topicProb = topicExpectation / words.size();
    topicParam.topicProb = topicProb;
  }
}

double expmxm::jointProbWordTopic(std::string& word, std::string& topic, std::map<std::string, expmxm::TopicParam>& parameters ) {
  expmxm::TopicParam& topicParam = parameters[topic];
  //std::cout << " Condition  word prob: " << word << " : "<<  topicParam.condWordProb[word] << " topic prob " << topicParam.topicProb << std::endl;
 
  double prob = topicParam.condWordProb[word] * topicParam.topicProb;
  return prob;
}


double expmxm::probOfWord(std::string& word, std::map<std::string, expmxm::TopicParam>& parameters) {
  double prob = 0;
  // std::cout <<  " Prob of Word: " << word << std::endl;
  for(std::map<std::string, expmxm::TopicParam>::iterator tpcIt = parameters.begin(); tpcIt != parameters.end(); ++tpcIt) {
    std::string topic = tpcIt->first;
    prob += expmxm::jointProbWordTopic(word, topic, parameters);
    //    std::cout << " prob " << prob ;
  } 
  //  std::cout << std::endl;
  return prob;
}

/**
 * We have to find the joint probabilty of hidden clas variable and the words.
 * Now we consider the the hidden variable of two types , one is the topic variable 
 * and the other is global variable. We consider each word is sample from one of the topic variables.
 * And each variable is sample independently.
 * http://cs.stackexchange.com/questions/10637/applying-expectation-maximization-to-coin-toss-examples
 */
double expmxm::likelihoodFunction(std::vector<std::string>& observations, std::map<std::string, expmxm::TopicParam>& parameters) {
  double likelihood = 0;
  for(std::vector<std::string>::iterator vecIt = observations.begin(); vecIt != observations.end(); ++vecIt) {
    std::string word = *vecIt;
    for(std::map<std::string, expmxm::TopicParam>::iterator mapIt = parameters.begin(); mapIt != parameters.end(); ++mapIt) {
      std::string topic = mapIt->first;
      double logWordProb = expmxm::jointProbWordTopic(word, topic, parameters);
      if(logWordProb <= 0)
        continue;
      //     std::cout << "Topic " << topic << " word " << word << " jnt prob " << logWordProb << std::endl;
      double condProb = expmxm::probTopicGivenWord(word, topic, parameters);
      logWordProb = (log(logWordProb) - log(condProb)) * condProb;
      likelihood += logWordProb;
     }
  }
  return likelihood;
}

void expmxm::initializeWordProb(expmxm::TopicParam& topicParam, unsigned long docSize) {
  for(std::map<std::string, double>::iterator mapIt = topicParam.lambdaEstimate.begin(); mapIt != topicParam.lambdaEstimate.end(); ++mapIt) {
    std::string term = mapIt->first;
    double lambda = mapIt->second * docSize;
    double freq = topicParam.freqMap[term];
    double prob =pow(lambda, freq) / exp(lambda);
    while(freq > 1) {
      prob = prob/freq;
      --freq;
    }
    topicParam.condWordProb[term] = prob;
  }
}


void expmxm::updateWordProb(std::vector<std::string>& words, std::map<std::string, expmxm::TopicParam>& parameters) {

  for(std::map<std::string, expmxm::TopicParam>::iterator mapIt = parameters.begin(); mapIt != parameters.end(); ++mapIt) {
    expmxm::TopicParam& topic = mapIt->second;
    expmxm::initializeWordProb(topic, words.size());
  }
}

void expmxm::printProb(std::vector<std::string>& text, expmxm::TopicParam& topic) {
  std::set<std::string> seenWords;
  for(std::vector<std::string>::iterator textIt = text.begin(); textIt != text.end(); ++textIt) {
    std::string word = *textIt;
    if(seenWords.find(word) != seenWords.end())
      continue;
    std::cout << "Word : " << word << " freq : " << topic.freqMap[word] << " prob: " <<  topic.condWordProb.at(word) << " topic prob " << topic.topicProb << std::endl;
    seenWords.insert(word);
  }
  //std::cout << std::endl ;
}

void expmxm::iteration(std::string& id, std::vector<std::string>& text, std::map<std::string, double>& collectionProb, std::map<std::string, double>& termProb) {

  unsigned long textSize = text.size();
  std::map<std::string, double> freqMap;

  for(std::vector<std::string>::iterator termIt = text.begin(); termIt != text.end(); ++termIt) 
    freqMap[*termIt]++;
 
  std::map<std::string, double> docProb;
  for(std::map<std::string, double>::iterator termIt = freqMap.begin(); termIt != freqMap.end(); ++termIt) {
    std::string term = termIt->first;
    //    double prob = termIt->second / textSize;
    docProb[term] = 1.0/ text.size();
  }

  std::map<std::string, expmxm::TopicParam> parameters;
  expmxm::TopicParam mainTopic;
  mainTopic.topic = id;
  mainTopic.topicProb = 0.5;
  mainTopic.lambdaEstimate = docProb;
  mainTopic.freqMap = freqMap;
  parameters[mainTopic.topic] = mainTopic;

  expmxm::TopicParam backTopic;
  backTopic.topic = "background";
  backTopic.topicProb = 1 - mainTopic.topicProb;
  backTopic.lambdaEstimate = collectionProb;
  backTopic.freqMap = freqMap;
  parameters[backTopic.topic] = backTopic;
   
  double score = 0; 
  double scorediff = 1000;
  //std::cout << score << std::endl;  
  //expmxm::printProb(text, parameters[id]);
  while(scorediff > 0.1) {

    expmxm::updateWordProb(text, parameters);  
    expmxm::estimateTopicCondProb(text, parameters);

    expmxm::maximizeLambda(text, parameters);
    expmxm::maximizeTopicProb(text, parameters);     

    double newscore = expmxm::likelihoodFunction(text, parameters); 
    //    std::cout << "First iteration " << newscore << std::endl;
    //expmxm::printProb(text, parameters[id]);
    scorediff = abs(newscore - score);
    score = newscore;
  } 

  //  std::cout << "Query Id :" << id << std::endl;
  expmxm::TopicParam& topic = parameters[id];
  for(std::map<std::string, double>::iterator mapIt = topic.condWordProb.begin(); mapIt != topic.condWordProb.end(); ++mapIt) {
    termProb[mapIt->first] = mapIt->second;    
    //    std::cout << "Term :" << mapIt->first  << " " << mapIt->second << std::endl;
  }
}

