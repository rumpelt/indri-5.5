#include "BM25Psg.hpp"

void BM25Psg::setCollectionSize(unsigned long collSize) {
  BM25Psg::_collectionSize = collSize;
}

void BM25Psg::setCollectionFreq(std::string term, unsigned long freq) {
  BM25Psg::_collectionFreq[term] = freq;
}

float BM25Psg::score(std::vector<std::string>& query, Passage* psg) {
  float score;

  for (std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string term = *qIt;
  }
  return score;
}

float BM25Psg::score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize) {
  return 0.0;
}
