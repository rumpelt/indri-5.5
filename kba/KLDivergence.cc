#include "KLDivergence.hpp"
#include <map>
#include <stdexcept>

using namespace kba::scorer;
using namespace kba::entity;
using namespace kba::term;
float KLDivergence::_alphaD = 10.0000;
float KLDivergence::_logAlphaD = log(_alphaD)/kba::term::LOG2;

KLDivergence::KLDivergence(std::vector<Entity*> entitySet, CorpusStat* crpStat, std::map<std::string, TermStat*> trmStatMap) : _entitySet(entitySet), _crpStat(crpStat), _trmStatMap(trmStatMap) {
}


float KLDivergence::score(kba::stream::ParsedStream* parsedStream, Entity* entity, int maxScore) {
  float docScore = 0;
  int docSize = parsedStream->size;
  long collectionSize = _crpStat->collectionSize;
  std::vector<std::string> query;
  float cutoff;
  if((entity->abstractTokens).size() > 0) {
    query = (entity->abstractTokens);
    cutoff = 700;
  }
  else {
    query = entity->labelTokens; // Okay we were not able to get the abstract text and so we just use the labeltokens
    cutoff = 500.0;
  }
  int querySize = query.size();

  for(std::vector<std::string>::iterator queryIt = query.begin(); queryIt != query.end(); ++queryIt) {
    std::string word = *queryIt;
    float queryFreq = 0.000000;
    try {
      queryFreq = (entity->textFreq).at(word);
    }
    catch(std::out_of_range& oor) {
      queryFreq = 1; // We were not able to get the abstract text and so we are using the label tokens and count of each query word is just 1.
    }

    float collFreq = 0;
    float docFreq = 0;
    float score=0.0000000;
    try {
      docFreq = (parsedStream->tokenFreq).at(word);
      score = log(docFreq * collectionSize);
    }
    catch(std::out_of_range& oor) {
    }

    try {
      collFreq = (_trmStatMap.at(word))->collFreq;
      if(collFreq > 0.99999) {
        score = (score - log(KLDivergence::_alphaD * docSize * collFreq))/ kba::term::LOG2;
        //std::cout << "Coll Freq " <<  score <<  " " << collFreq << "\n";
      }
    }
    catch(std::out_of_range& oor) {
    }
   
    score = (score * queryFreq) / querySize;
    score = score + KLDivergence::_logAlphaD;
    docScore += score;
  }
  //  std::cout << "Entity " << entity->wikiURL << " score " << docScore << "\n";
  return docScore > cutoff ? docScore : 0;
}
