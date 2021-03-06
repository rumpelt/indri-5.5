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
    double collFreq = (trmIt->second)->collFreq * _mu;
    collFreq = collFreq / _crpStat->collectionSize; // _mu factor for dirichlet smoothing.
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
  for(std::vector<std::string>::iterator termIt = (entity->labelTokens).begin(); termIt != (entity->labelTokens).end(); ++termIt) {
    std::string term = *termIt;
    if( (parsedStream->langModelProb).find(term) == (parsedStream->langModelProb).end()) {
      float termFreq = 0;
      if((parsedStream->tokenFreq).find(term) != (parsedStream->tokenFreq).end())
        termFreq = (parsedStream->tokenFreq)[term];
      double collFreq = _collFreqMap[term]; // here coll Prob is factored by mu and corpus collectionSize
      double totalFreq = termFreq  + collFreq;
      if (totalFreq > 0.0001) // This is based on 1/_mu (1/2500). We want at least that much value. We are trying to induce some counts
        totalFreq  = log(totalFreq);
      else
        totalFreq = 0;
      float score = totalFreq - log((parsedStream->size + _mu)); 
      (parsedStream->langModelProb).insert(std::pair<std::string, float>(term, score));
    }
    docScore = docScore + (parsedStream->langModelProb)[term]; // _mu factor for collection probability is already taken care of in computeCollection
  }

  return docScore;
}
