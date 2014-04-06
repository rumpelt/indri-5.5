#include "LanguageModelPsg.hpp"
#include <map>
#include <math.h>
#include <stdio.h>
#include <iostream>

LanguageModelPsg::LanguageModelPsg() : _mu(2500) {}
LanguageModelPsg::LanguageModelPsg(float mu) : _mu(2500) {}

void LanguageModelPsg::setCollectionSize(unsigned long collSize) {
  //  std::cout << "Setting coll size " << collSize << std::endl;
  LanguageModelPsg::_collectionSize = collSize;
}

void LanguageModelPsg::setCollectionFreq(std::string term, unsigned long freq) {
  LanguageModelPsg::_collectionFreq[term] =  freq;
}

float LanguageModelPsg::score(std::vector<std::string>& query, Passage* psg) {
  float docScore = 0;
  float docSz = log((psg->getPsgSz() + LanguageModelPsg::_mu)); // there is paper which consider _mu making dependent on the terms, (Modelling document burstiness by dirclet allocation)
  
  for(std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string word = *qIt;
    int docFreq = psg->freq(word);
    unsigned long collFreq = LanguageModelPsg::_collectionFreq[word];
    if(docFreq == 0 && collFreq == 0)
      collFreq = 1;
    float cFreq = (LanguageModelPsg::_mu  * collFreq) / LanguageModelPsg::_collectionSize;
    //    std::cout << " " << word << " : freq : " << psg->freq(word); 
    float docCount = docFreq + cFreq;
    docScore = docScore + log(docCount) - docSz;
  }

  return docScore;
}

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
