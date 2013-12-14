#include "LanguageModel.hpp"
#include <map>
#include <math.h>
#include <stdio.h>
#include <iostream>
LanguageModel::LanguageModel() : _mu(100) {}

float LanguageModel::score(std::vector<std::string> query, Passage* psg, std::map<std::string, unsigned long> collFreq, const unsigned long collSize) {
  float docScore =0;
  //  std::cout << "passge sz " << psg->getPsgSz() << " Coll size " << collSize << "\n";
  //  float logCollSz = log(collSize);
  float docSz = log((psg->getPsgSz() + LanguageModel::_mu)); // there is paper which consider _mu making dependent on the terms, (Modelling document burstiness by dirclet allocation)
  for(std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string word = *qIt;
    float docFreq = psg->freq(word);
    float cFreq = (LanguageModel::_mu  * collFreq[word]) / collSize;
    //    std::cout << " " << word << " : freq : " << psg->freq(word); 
    docFreq = docFreq + cFreq;
    docScore = docScore + log(docFreq) - docSz;
  }
  //  std::cout << "\n";
   return docScore;
}
