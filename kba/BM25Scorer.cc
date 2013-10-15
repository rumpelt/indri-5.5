#include <BM25Scorer.hpp>
#include <stdexcept>
#include <iostream>
#include <Logging.hpp>

kba::scorer::BM25Scorer::BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  boost::shared_ptr<kba::term:: TermBase> termBase, int maxScore) : _entitySet(entitySet), _termBase(termBase), _maxScore(maxScore), _parameterK(1.75), _parameterB(0.75) {
}

   
/**
 * Only visually tested.
 */
float kba::scorer::BM25Scorer::computeQueryTermIDF(std::string term) {
  kba::term::TermBase* termBase = _termBase.get();
  float idf = 0.0;
  try {
    idf = (termBase->logIDF).at(term);
  }
  catch(std::out_of_range& oor) {
    //std::cout << "Could not find the term in the dictionary: "+ term + "\n";
  } 
  return idf;
}

float kba::scorer::BM25Scorer::computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms) {
  using namespace kba::scorer;

  float  maxDocScore = 0.0;
  float docScore = 0.0;
  kba::term::TermBase* termBase = BM25Scorer::_termBase.get();

  float normalDocLength = (stream->tokens).size() /  termBase->avgDocLength; // normalized document length (dl / avgdl)
  float maxDocLength = queryTerms.size()/ termBase->avgDocLength; // If the document just contains the query terms then we hit the jackspot and this is the ideal condition and we cannot score mor than this.
  
  
  float k1b = BM25Scorer::_parameterK * BM25Scorer::_parameterB; // the factor k1 * b
  float k1minusB = BM25Scorer::_parameterK * (1 - BM25Scorer::_parameterB); // the factor k1 * (1 - b)
  
  float denominatorFactor = k1minusB + normalDocLength * k1b;
  float minDenominatorFactor = k1minusB + maxDocLength * k1b;
  
  if(denominatorFactor < minDenominatorFactor)
    minDenominatorFactor = denominatorFactor; // This case possible if the document is very very short , for example just one token.

  for(std::vector<std::string>::iterator queryIt = queryTerms.begin(); queryIt != queryTerms.end(); queryIt++) {
    try {
      
      int freq = (stream->tokenFreq).at(*queryIt);
 
      float idf = BM25Scorer::computeQueryTermIDF(*queryIt);

      docScore = docScore + (idf * (freq / (freq + denominatorFactor)));
      maxDocScore = maxDocScore + (idf * (freq / (freq + minDenominatorFactor)));
      //     std::cout << "avgDoc lenght : " << termBase->avgDocLength << " ";
      //std::cout << "idf :" << idf << " docscore : " << docScore << " max : " << maxDocScore << "\n";
    } catch(std::out_of_range& oor) {
      //      (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, value)); 
    }    
  }
  //    std::cout << "Full score " << maxDocScore << " Actual Score " << docScore << "\n";
  return maxDocScore > 0.0 ? docScore/maxDocScore : 0.0;
}


float kba::scorer::BM25Scorer::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  if ((entity->labelTokens).size() <= 0) {
    std::cout << "Not a vailid entiy " << entity->wikiURL << "\n";
    return 0;
  }
  using namespace boost;
  using namespace kba::entity;

  float score= 0.0;
  float maxEntityScore = 70.0 * maxScore / 100;

  score = maxEntityScore * kba::scorer::BM25Scorer::computeNormalizedDocScore(parsedStream, entity->labelTokens);
  
  float maxRelatedScore = maxScore - maxEntityScore;
  float maxRelStep = maxRelatedScore / (entity->relatedEntities).size();
  for (std::vector<shared_ptr<Entity> >::iterator relEntIt = (entity->relatedEntities).begin();   relEntIt != (entity->relatedEntities).end(); ++relEntIt) {
    Entity* related = (*relEntIt).get();
    float relatedScore = kba::scorer::BM25Scorer::computeNormalizedDocScore(parsedStream, related->labelTokens);
    score = score + relatedScore * maxRelStep;
  }

  return score;   
}

int kba::scorer::BM25Scorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int cutOffScore) {
  return -1;
}
