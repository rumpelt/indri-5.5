#include "KLDivergence.hpp"
#include <map>
#include <stdexcept>

using namespace kba::scorer;
using namespace kba::entity;
using namespace kba::term;
float KLDivergence::_mu = 2500;

KLDivergence::KLDivergence(std::vector<Entity*> entitySet, CorpusStat* crpStat, std::map<std::string, TermStat*> trmStatMap) : _entitySet(entitySet), _crpStat(crpStat), _trmStatMap(trmStatMap) {
  computeCollectionProb();
  //  computeMaxDocScore();
}

void kba::scorer::KLDivergence::computeMaxDocScore() {

  for(std::vector<kba::entity::Entity*>::iterator entIt = _entitySet.begin(); entIt != _entitySet.end(); ++entIt) {
    kba::entity::Entity* entity = *entIt;
    std::map<std::string, int>& queryMap = entity->textFreq;
    int querySize = (entity->abstractTokens).size();
    if(querySize <= 0) {
      queryMap = entity->labelMap; // Okay we were not able to get the abstract text and so we just use the labeltokens
      querySize = (entity->labelTokens).size();
    }
    float docScore = 0;

    for(std::map<std::string, int>::iterator queryIt = queryMap.begin(); queryIt != queryMap.end(); ++queryIt) {
      std::string word = queryIt->first;
      float docFreq = queryIt->second;
      float collFreq = _collFreqMap[word];
      docFreq = (docFreq * _crpStat->collectionSize) + collFreq;
      float logFactor = log(docFreq) - log ((querySize + _mu) * _crpStat->collectionSize);
      logFactor = ((queryIt->second) * logFactor) / querySize;
      docScore = docScore + logFactor; 
    }
    //    docScore = -1 * docScore;
    //  std::cout << "Entity " << entity->wikiURL << " score " << docScore << "\n";
    _maxScores.insert(std::pair<std::string, float>(entity->wikiURL, docScore));
  }
}

float KLDivergence::score(kba::stream::ParsedStream* parsedStream, Entity* entity, int maxScore) {

  //  float cutoff = -4;
  int querySize = (entity->abstractTokens).size();
  std::map<std::string, int>& queryMap = entity->textFreq;
  if(querySize <= 0) {
    queryMap = entity->labelMap; // Okay we were not able to get the abstract text and so we just use the labeltokens
    querySize = (entity->labelTokens).size();
  }

  float docScore = 0;

  for(std::map<std::string, int>::iterator queryIt = queryMap.begin(); queryIt != queryMap.end(); ++queryIt) {
    std::string word = queryIt->first;
    float docFreq = 0;
    try {
      docFreq = (parsedStream->tokenFreq).at(word);
    }
    catch(const std::out_of_range& oor) {
    }
    float collFreq = _collFreqMap[word];
    
    docFreq = (docFreq * _crpStat->collectionSize) + collFreq;
    if (docFreq >= 1.0)
      docFreq = log(docFreq);
    float logFactor = docFreq - log ((parsedStream->size + _mu) * _crpStat->collectionSize);
    logFactor = ((queryIt->second) * logFactor) / querySize;
    docScore = docScore + logFactor; 
  }

  return docScore;
}

void kba::scorer::KLDivergence::computeCollectionProb() {

  for(std::map<std::string, TermStat*>::iterator trmIt = _trmStatMap.begin(); trmIt != _trmStatMap.end(); ++trmIt) {
    std::string term = (trmIt)->first;
    float collFreq = (trmIt->second)->collFreq * _mu; // _mu factor for dirichlet smoothing.
    _collFreqMap.insert(std::pair<std::string, float>(term, collFreq));
  }

}
