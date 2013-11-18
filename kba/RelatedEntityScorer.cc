#include "RelatedEntityScorer.hpp"
#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include <stdexcept>
#include <iostream>
#include <stdexcept>
#include "Tokenize.hpp"
#include "cassert"
 

int kba::scorer::RelatedEntityScorer::score(streamcorpus::StreamItem* streamItem, kba::entity::Entity* entity, int maxScore) {
  return -1;
}


float kba::scorer::RelatedEntityScorer::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  
  assert((entity->labelTokens).size() > 0);


  std::vector<boost::shared_ptr<kba::entity::Entity> > relatedEntities = entity->relatedEntities;
  
  float score = 0;
  float mainMaxScore = (float)maxScore * 70.0 / 100.0;
  float relatedMaxScore = maxScore - mainMaxScore;
  
  float mainScoreStep = mainMaxScore / (entity->labelTokens).size();
  
  float scorePerEntity = 0;
  if(relatedEntities.size() > 0)
    scorePerEntity = relatedMaxScore / relatedEntities.size();

  for (int idx=0; idx < (entity->labelTokens).size() ; idx++) {
    std::string token = (entity->labelTokens)[idx];
    if ((parsedStream->tokenSet).count(token) > 0)
      score = score + mainScoreStep;
  }        

  if(score > mainMaxScore)
    score = mainMaxScore;


  for(int idx=0; idx < relatedEntities.size(); idx++) {
    kba::entity::Entity* relatedEnt = relatedEntities[idx].get();
    float step = scorePerEntity / (relatedEnt->labelTokens).size();    

    for (int index=0; index < (relatedEnt->labelTokens).size() ; index++) {
      std::string token = (relatedEnt->labelTokens)[index];
      if ((parsedStream->tokenSet).count(token) > 0)
        score = score + step;
    }
  }

  return score > (float)maxScore  ? (float)maxScore : score;
}

std::vector<kba::entity::Entity*> kba::scorer::RelatedEntityScorer::getEntityList() {
  return _entitySet;
}

kba::scorer::RelatedEntityScorer::RelatedEntityScorer(std::vector<kba::entity::Entity*> entitySet, std::map<std::string, std::string> repoMap, int maxScore) : _repoMap(repoMap), _entitySet(entitySet), _maxScore(maxScore){

}

std::string kba::scorer::RelatedEntityScorer::getModelName() {
  return  "RelatedEntity";
}
