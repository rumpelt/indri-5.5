#include "LanguageModel.hpp"
#include <map>
#include <math.h>
#include <stdio.h>
#include <iostream>
LanguageModel::LanguageModel() : _mu(2500) {}
LanguageModel::LanguageModel(float mu) : _mu(mu) {}
float LanguageModel::score(std::vector<std::string> query, Passage* psg, std::map<std::string, unsigned long> collFreq, const unsigned long collSize) {
  float docScore =0;
  //  std::cout << "passge sz " << psg->getPsgSz() << " Coll size " << collSize << "\n";
  //  float logCollSz = log(collSize);
  float docSz = log((psg->getPsgSz() + LanguageModel::_mu)); // there is paper which consider _mu making dependent on the terms, (Modelling document burstiness by dirclet allocation)
  for(std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string word = *qIt;
    float docFreq = psg->freq(word);
    int cFreq =  collFreq[word];
    if(cFreq == 0)
      cFreq = 1;
    float cProb = (LanguageModel::_mu  * collFreq[word]) / collSize;
    if(cProb <= 0.0000000000) {
      std::cout << "LanguageModel : the computed collection probability is less than equal to zero . please check. It might be stopword" << std::endl;
      continue;
    }
    //    std::cout << " " << word << " : freq : " << psg->freq(word) << " cFreq " << cFreq << " coll size " << collSize << " collFreq" << collFreq[word] <<"\n" ; 
    docFreq = docFreq + cProb;
    docScore = docScore + log(docFreq) - docSz;
  }
  //  std::cout << "\n";
   return docScore;
}
