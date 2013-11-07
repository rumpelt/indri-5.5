#include <BM25Scorer.hpp>
#include <stdexcept>
#include <iostream>
#include <Logging.hpp>

kba::scorer::BM25Scorer::BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::set<kba::term::TermStat*> trmStat,   int maxScore) : _entitySet(entitySet), _crpStat(crpStat), _trmStat(trmStat),_maxScore(maxScore), _parameterK(1.75), _parameterB(0.75) {
  _k1b = BM25Scorer::_parameterK * BM25Scorer::_parameterB; // the factor k1 * b
  _k1minusB = BM25Scorer::_parameterK * (1 - BM25Scorer::_parameterB); // the factor k1 * (1 - b) 
  computeLogIDF();
  computeMaxDocScores();

}

   
void kba::scorer::BM25Scorer::computeLogIDF() {
  for(std::set<kba::term::TermStat*>::iterator tIt = _trmStat.begin(); tIt != _trmStat.end(); ++tIt) {
    std::string term = (*tIt)->term;
    long docFreq = (*tIt)->docFreq;
    float idf = log((_crpStat->totalDocs - docFreq + 0.5)/ (docFreq + 0.5));
    idf = idf / kba::term::LOG2;
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
      float idf = _idf.at(term);
      //std::cout << " Term: " << term << " idf " << idf << "\n";
      maxDocScore = maxDocScore + (idf * (freq / (freq + denominatorFactor)));
    }  
    //std::cout << ent->wikiURL <<  " BM25 Max Doc Score " << maxDocScore << " denominatorFactor " << denominatorFactor << " max doc length " <<  maxDocLength << "\n";
    _maxScores.insert(std::pair<std::string, float>(ent->wikiURL, maxDocScore));
  }
}

float kba::scorer::BM25Scorer::computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, float maxDocScore) {
  using namespace kba::scorer;
  float docScore = 0.0;
  float normalDocLength = (float)(stream->size) /  _crpStat->averageDocSize; // normalized document length (dl / avgdl)
  float denominatorFactor = _k1minusB + normalDocLength * _k1b;

  for(std::vector<std::string>::iterator queryIt = queryTerms.begin(); queryIt != queryTerms.end(); queryIt++) {
    try {
      int freq = (stream->tokenFreq).at(*queryIt);
      float idf = _idf.at(*queryIt);
      docScore = docScore + (idf * (freq / (freq + denominatorFactor)));
    } catch(std::out_of_range& oor) {
      //      (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, value)); 
    }    
  }
  //  std::cout << " Normalzed doc length " << normalDocLength  <<" Computed doc score " << docScore << " strema size " << stream->size << " Corpus Avg doc " << _crpStat->averageDocSize << " Max Score " << maxDocScore <<  "\n";
  return maxDocScore > 0.0 ? docScore/maxDocScore : 0.0;
}


float kba::scorer::BM25Scorer::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  if ((entity->labelTokens).size() <= 0) {
    std::cout << " Not a vailid entiy " << entity->wikiURL << "\n";
    return 0;
  }
  using namespace boost;
  using namespace kba::entity;
  //  std::cout << " Entity " << entity->wikiURL <<"\n";
  float score = maxScore * kba::scorer::BM25Scorer::computeNormalizedDocScore(parsedStream, entity->labelTokens, _maxScores.at(entity->wikiURL));
  return score;   
}

int kba::scorer::BM25Scorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int cutOffScore) {
  return -1;
}
