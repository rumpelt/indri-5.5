#include <BM25Scorer.hpp>
#include <stdexcept>
#include <iostream>
#include <Logging.hpp>

kba::scorer::BM25Scorer::BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::map<std::string, kba::term::TermStat*> trmStatMap,  float cutoffScore,  int maxScore) : _entitySet(entitySet), _crpStat(crpStat), _trmStatMap(trmStatMap), _cutoffScore(cutoffScore), _maxScore(maxScore), _parameterK(1.75), _parameterB(0.75) {
  _k1b = BM25Scorer::_parameterK * BM25Scorer::_parameterB; // the factor k1 * b
  _k1minusB = BM25Scorer::_parameterK * (1 - BM25Scorer::_parameterB); // the factor k1 * (1 - b) 
  _denominatorFactor = -1;
  computeLogIDF();
  //  computeMaxDocScores();
}

   
void kba::scorer::BM25Scorer::computeLogIDF() {
  for(std::map<std::string, kba::term::TermStat*>::iterator tIt = _trmStatMap.begin(); tIt != _trmStatMap.end(); ++tIt) {
    std::string term = tIt->first;
    long docFreq = (tIt->second)->docFreq;
    float idf = log((_crpStat->totalDocs - docFreq + 0.5)/ (docFreq + 0.5));
    idf = idf;
    //std::cout << "Term " << term << " idf " << idf << " doc Freq " << docFreq << " Total doc " << _crpStat->totalDocs << "\n";
    _idf.insert(std::pair<std::string, float>(term, idf));
  }
}


void kba::scorer::BM25Scorer::computeMaxDocScores() {
  using namespace kba::entity;
  using namespace kba::term;

  for(std::vector<Entity*>::iterator eIt=_entitySet.begin() ; eIt != _entitySet.end() ; ++eIt) {
    Entity* ent = *eIt;
    float maxDocScore  = 0.0;
    float maxDocLength = (ent->labelTokens).size() / _crpStat->averageDocSize;
    float denominatorFactor = _k1minusB + maxDocLength * _k1b;
    //    std::cout << " Entity " << ent->wikiURL << "\n";
    for(std::vector<std::string>::iterator queryIt = (ent->labelTokens).begin(); queryIt != (ent->labelTokens).end(); ++queryIt) {
      std::string term = *queryIt;
      int freq = 1; // The freq of each term in the ideal document. the ideal document is the entity itself. It is assumend that each term will appear only once in the entity.
      float idf = _idf[term];
      //std::cout << " Term: " << term << " idf " << idf << "\n";
      maxDocScore = maxDocScore + (idf * (freq / (freq + denominatorFactor)));
    }  
    //std::cout << ent->wikiURL <<  " BM25 Max Doc Score " << maxDocScore << " denominatorFactor " << denominatorFactor << " max doc length " <<  maxDocLength << "\n";
    _maxScores.insert(std::pair<std::string, float>(ent->wikiURL, maxDocScore));
  }
}

float kba::scorer::BM25Scorer::computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, float maxDocScore) {
  using namespace kba::scorer;

  float docScore = 0;
  
  for(std::vector<std::string>::iterator queryIt = queryTerms.begin(); queryIt != queryTerms.end(); queryIt++) {
    try {
      std::string term= *queryIt;
      if((stream->bm25Prob).find(term) == (stream->bm25Prob).end()) {
        int freq = (stream->tokenFreq).at(term);
        float idf = _idf.at(*queryIt);
        if(_denominatorFactor < 0) {
          float normalDocLength = (float)(stream->size) /  _crpStat->averageDocSize; // normalized document length (dl / avgdl)
          _denominatorFactor = _k1minusB + normalDocLength * _k1b;
        }
        float score = idf * (freq / (freq + _denominatorFactor));
        (stream->bm25Prob).insert(std::pair<std::string, float>(term, score));
      }
      docScore = docScore + (stream->bm25Prob)[term];
    } catch(std::out_of_range& oor) {

    }    
  }
  return docScore;
}



