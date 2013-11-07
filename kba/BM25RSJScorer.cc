#include <BM25RSJScorer.hpp>
#include <stdexcept>
#include <iostream>
#include <Logging.hpp>

kba::scorer::BM25RSJScorer::BM25RSJScorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::set<kba::term::TermStat*> trmStat,  std::set<kba::term::TopicTerm*> tpcTrm, int maxScore) : _entitySet(entitySet), _crpStat(crpStat), _trmStat(trmStat), _tpcTrm(tpcTrm), _maxScore(maxScore), _parameterK(1.75), _parameterB(0.75) {
  _k1b = BM25RSJScorer::_parameterK * BM25RSJScorer::_parameterB; // the factor k1 * b
  _k1minusB = BM25RSJScorer::_parameterK * (1 - BM25RSJScorer::_parameterB); // the factor k1 * (1 - b) 
  computeLogRSJWeight();
  computeMaxDocScores();
}

void kba::scorer::BM25RSJScorer::computeLogRSJWeight() {
  using namespace kba::term;
  for(std::set<kba::term::TopicTerm*>::iterator tIt = _tpcTrm.begin(); tIt != _tpcTrm.end(); ++tIt) {
    TopicTerm* tpcT = *tIt;
    std::string topic = tpcT->topic;
    std::string term = tpcT->term;
    float  relDoc = (tpcT->relevantDocFreq + 0.5);
    float logRelDoc = log(relDoc);
    float collRel =  (_crpStat->judgedSample - tpcT->relevantDocFreq - tpcT->judgedDocFreq + tpcT->relevantDocFreq + 0.5);
    float logCollRel = log(collRel);
    float numerator = logRelDoc + logCollRel;
 
    float unjudged  = tpcT->judgedDocFreq - tpcT->relevantDocFreq + 0.5;
    float logUnjudged = log(unjudged);
    float compRelSet = tpcT->relevantSetSize - tpcT->relevantDocFreq + 0.5;
    float logCompRelSet = log(compRelSet);
    float denominator  = logUnjudged + logCompRelSet;
    
    float weight = (numerator - denominator) / kba::term::LOG2;
    
    _rsj.insert(std::pair<std::string, float>(topic+term, weight));
  }
}


void kba::scorer::BM25RSJScorer::computeMaxDocScores() {
  using namespace kba::entity;
  using namespace kba::term;

  for(std::vector<Entity*>::iterator eIt=_entitySet.begin() ; eIt != _entitySet.end() ; ++eIt) {
    Entity* ent = *eIt;
    float maxDocScore  = 0.0;
    float maxDocLength = (ent->labelTokens).size() / _crpStat->averageDocSize;
    float denominatorFactor = _k1minusB + maxDocLength * _k1b;
    std::cout << "Entity " << ent->wikiURL << "\n";
    for(std::vector<std::string>::iterator queryIt = (ent->labelTokens).begin(); queryIt != (ent->labelTokens).end(); ++queryIt) {
      std::string term = *queryIt;
      int freq = 1; // The freq of each term in the ideal document. the ideal document is the entity itself. It is assumend that each term will appear only once in the entity.
      float idf = _rsj.at(ent->wikiURL+term);
      std::cout << "Term: " << term << " idf " << idf << "\n";
      maxDocScore = maxDocScore + (idf * (freq / (freq + denominatorFactor)));
    }  
    std::cout << ent->wikiURL <<  "BM25 Max Doc Score " << maxDocScore << " denominatorFactor " << denominatorFactor << " max doc length " <<  maxDocLength << "\n";
    _maxScores.insert(std::pair<std::string, float>(ent->wikiURL, maxDocScore));
  }
}

float kba::scorer::BM25RSJScorer::computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, std::string entity, float maxDocScore) {
  using namespace kba::scorer;
  float docScore = 0.0;
  float normalDocLength = (stream->tokens).size() /  _crpStat->averageDocSize; // normalized document length (dl / avgdl)
  float denominatorFactor = _k1minusB + normalDocLength * _k1b;

  for(std::vector<std::string>::iterator queryIt = queryTerms.begin(); queryIt != queryTerms.end(); queryIt++) {
    try {
      int freq = (stream->tokenFreq).at(*queryIt);
      float idf = _rsj.at(entity + *queryIt);
      docScore = docScore + (idf * (freq / (freq + denominatorFactor)));
    } catch(std::out_of_range& oor) {
      //      (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, value)); 
    }    
  }
  std::cout << "Normalzed doc length " << normalDocLength  <<"Computed doc score " << docScore << "\n";
  return maxDocScore > 0.0 ? docScore/maxDocScore : 0.0;
}


float kba::scorer::BM25RSJScorer::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  if ((entity->labelTokens).size() <= 0) {
    std::cout << "Not a vailid entiy " << entity->wikiURL << "\n";
    return 0;
  }
  using namespace boost;
  using namespace kba::entity;
  std::cout << "Entity " << entity->wikiURL <<"\n";
  float score = maxScore * kba::scorer::BM25RSJScorer::computeNormalizedDocScore(parsedStream, entity->labelTokens, entity->wikiURL, _maxScores.at(entity->wikiURL));
  return score;   
}

int kba::scorer::BM25RSJScorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int cutOffScore) {
  return -1;
}
