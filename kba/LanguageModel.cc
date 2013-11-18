#include "LanguageModel.hpp"
#include "limits.h"
#include "float.h"
using namespace kba::scorer;
using namespace kba::term;

kba::scorer::LanguageModel::LanguageModel(std::vector<kba::entity::Entity*>& entitySet, std::map<std::string, kba::term::TermStat*> trmStatMap, kba::term::CorpusStat* crpStat, float cutoffScore, float mu) : _entitySet(entitySet), _trmStatMap(trmStatMap), _crpStat(crpStat), _cutoffScore(cutoffScore), _mu(mu) {
  computeCollectionProb();
  //  computeMaxDocScore();
}

std::string kba::scorer::LanguageModel::getModelName() {
  return "LanguageModel";
}
std::vector<kba::entity::Entity* > kba::scorer::LanguageModel::getEntityList() {
  return _entitySet;
}

void kba::scorer::LanguageModel::computeCollectionProb() {

  for(std::map<std::string, TermStat*>::iterator trmIt = _trmStatMap.begin(); trmIt != _trmStatMap.end(); ++trmIt) {
    std::string term = (trmIt)->first;
    float collFreq = (trmIt->second)->collFreq * _mu; // _mu factor for dirichlet smoothing.
    _collFreqMap.insert(std::pair<std::string, float>(term, collFreq));
  }

}
   
void kba::scorer::LanguageModel::computeMaxDocScore() {
  for(std::vector<kba::entity::Entity*>::iterator entIt = _entitySet.begin(); entIt != _entitySet.end(); ++entIt) {
    kba::entity::Entity* entity = *entIt;
    float docScore = 0.0;
    float docSizeLog = log((_crpStat->averageDocSize + _mu) * _crpStat->collectionSize);
    for(std::vector<std::string>::iterator termIt = (entity->labelTokens).begin(); termIt != (entity->labelTokens).end(); ++termIt) {
      std::string term = *termIt;
      float docFreq = 1 * _crpStat->collectionSize; // the document here query itself and so the freq is just 1
      float collFreq = _collFreqMap.at(term);
      docFreq = log(docFreq +collFreq);
      float score = (docFreq - docSizeLog);
      docScore = docScore + score; 
    }
    //    std::cout << "Max score "<< entity->wikiURL << " " << docScore << "\n";
    _maxScores.insert(std::pair<std::string, float>(entity->wikiURL, docScore));
  }
}
 
float kba::scorer::LanguageModel::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  float docScore = 0;
  float docSizeLog =log((parsedStream->size + _mu) * _crpStat->collectionSize);
  for(std::vector<std::string>::iterator termIt = (entity->labelTokens).begin(); termIt != (entity->labelTokens).end(); ++termIt) {
    std::string term = *termIt;
    float termFreq = 0;
    if((parsedStream->tokenFreq).find(term) != (parsedStream->tokenFreq).end())
      termFreq = (parsedStream->tokenFreq).at(term);
    float collFreq = _collFreqMap.at(term); // here coll Prob is factored by mu
    float totalFreq = termFreq * _crpStat->collectionSize + collFreq;
    if (totalFreq > 0.999999)
      totalFreq  = log(totalFreq);
    else
      totalFreq = 0;
    float score = (totalFreq - docSizeLog); 
    docScore = docScore + score; // _mu factor for collection probability is already taken care of in computeCollection
  }

  return docScore;
}
