#include "LanguageModel.hpp"

using namespace kba::scorer;
using namespace kba::term;

kba::scorer::LanguageModel::LanguageModel(std::set<kba::entity::Entity*>& entitySet, std::set<kba::term::TermStat*> trmStat, float mu) : _entitySet(entitySet), _trmStat(trmStat), _mu(mu) {
  computeCollectionProb();
}


void kba::scorer::LanguageModel::computeCollectionProb() {
  for(std::set<TermStat*>::iterator trmIt = _trmStat.begin(); trmIt != _trmStat.end(); ++trmIt) {
    std::string term = (*trmIt)->term;
    long colFreq = (*trmIt)->collFreq;
    float collProb =(_mu * colFreq)/ (_crpStat->totalDocs * _crpStat->averageDocSize) ; // _mu factor for dirichlet smoothing.
    std::cout << "Language model Term :: " << term << " Freq " << colFreq << " " << collProb <<  " Total docs " << _crpStat->totalDocs << " Average docsize" << _crpStat->averageDocSize << "\n";
    _collFreqMap.insert(std::pair<std::string, float>(term, collProb));
  }
}

void kba::scorer::LanguageModel::computeMaxDocScore() {
  for(std::set<kba::entity::Entity*>::iterator entIt = _entitySet.begin(); entIt != _entitySet.end(); ++entIt) {
    kba::entity::Entity* entity = *entIt;
    float docScore = 0.0;
    int docSize = (entity->labelTokens).size();
    for(std::vector<std::string>::iterator termIt = (entity->labelTokens).begin(); termIt != (entity->labelTokens).end(); ++termIt) {
      std::string term = *termIt;
      int termFreq = 1;
      float collProb = _collFreqMap.at(term);
      docScore = docScore + ((termFreq + collProb) / (docSize + _mu)); // _mu factor for collection probability is already taken care of in computeCollection
    }
    _maxScoreMap.insert(std::pair<std::string, float>(entity->wikiURL, docScore));
  }
}

float kba::scorer::LanguageModel::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  float docScore = 0.0;
  int docSize = parsedStream->size;
  for(std::vector<std::string>::iterator termIt = (entity->labelTokens).begin(); termIt != (entity->labelTokens).end(); ++termIt) {
    std::string term = *termIt;
    
    int termFreq = 0;
    if((parsedStream->tokenFreq).find(term) != (parsedStream->tokenFreq).end())
      termFreq = (parsedStream->tokenFreq).at(term);
    float collProb = _collFreqMap.at(term);
    docScore = docScore + ((termFreq + collProb) / (docSize + _mu)); // _mu factor for collection probability is already taken care of in computeCollection
  }
  float maxDocScore = _maxScoreMap.at(entity->wikiURL);
  if(maxDocScore > 0.0)
    docScore = (docScore/maxDocScore) *  maxScore;
  return docScore;
}
