#include "LanguageModelPsg.hpp"
#include <map>
#include <math.h>
#include <stdio.h>
#include <iostream>
LanguageModelPsg::LanguageModelPsg() : _mu(2500) {}

float LanguageModelPsg::score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize) {
  float docScore =0;
  //  std::cout << "passge sz " << psg->getPsgSz() << " Coll size " << collSize << "\n";
  //  float logCollSz = log(collSize);
  float docSz = log((psg->getPsgSz() + LanguageModelPsg::_mu)); // there is paper which consider _mu making dependent on the terms, (Modelling document burstiness by dirclet allocation)
  for(std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string word = *qIt;
    float docFreq = psg->freq(word);
    float cFreq = (LanguageModelPsg::_mu  * collFreq[word]) / collSize;
    //    std::cout << " " << word << " : freq : " << psg->freq(word); 
    docFreq = docFreq + cFreq;
    docScore = docScore + log(docFreq) - docSz;
  }
  //  std::cout << "\n";
   return docScore;
}


float LanguageModelPsg::score(std::vector<std::string>& query, Passage* psg, std::map<std::string, TermStat*>& termStatMap, CorpusStat* corpusStat) {
  float docScore = 0;
  float docSz = log((psg->getPsgSz() + LanguageModelPsg::_mu)); // there is paper which consider _mu making dependent on the terms, (Modelling document burstiness by dirclet allocation)
  for(std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string word = *qIt;
    double docFreq = psg->freq(word);
    double cFreq = (LanguageModelPsg::_mu  * (termStatMap[word])->docFreq) / (double)(corpusStat->totalDocs);
    //    std::cout << "Term " << word << " DF: " << docFreq << " col " << (termStatMap[word])->docFreq <<  " coll size " << corpusStat->totalDocs << " cfreq " << cFreq << "\n";
    docFreq = docFreq + cFreq;
    docScore = docScore + log(docFreq) - docSz; 
  }
  return docScore; 
}
