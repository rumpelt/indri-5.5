#include "KLDivergence.hpp"
#include <map>
#include <stdexcept>

using namespace kba::scorer;
using namespace kba::entity;
using namespace kba::term;
float KLDivergence::_alphaD = 1000.0000;
float KLDivergence::_logAlphaD = log(1000.0000)/kba::term::LOG2;

KLDivergence::KLDivergence(std::vector<Entity*> entitySet, CorpusStat* crpStat, std::map<std::string, TermStat*> trmStatMap) : _entitySet(entitySet), _crpStat(crpStat), _trmStatMap(trmStatMap) {
}


float KLDivergence::score(kba::stream::ParsedStream* parsedStream, Entity* entity, int maxScore) {
  float docScore = 0;
  int docSize = parsedStream->size;
  int querySize = (entity->abstractTokens).size();
  long collectionSize = _crpStat->collectionSize;
  for(std::map<std::string, float>::iterator queryIt = (entity->textFreq).begin(); queryIt != (entity->textFreq).end(); ++queryIt) {
    std::string word = queryIt->first;
    float queryFreq = queryIt->second;
    float collFreq = 0;
    float docFreq = 0;
    float score=0.0000000;
    try {
      docFreq = (parsedStream->tokenFreq).at(word);
   
      score = log(docFreq * collectionSize);
      //std::cout << "docFreq " << score  << " " << docFreq << "\n";
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
   
    score = score * queryFreq / querySize;
    score = score + KLDivergence::_logAlphaD;
    docScore += score;
  }
  //  std::cout << "Entity " << entity->wikiURL << " score " << docScore << "\n";
  return docScore;
}
