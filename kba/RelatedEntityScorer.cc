#include "RelatedEntityScorer.hpp"
#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include <stdexcept>
#include <iostream>
#include <stdexcept>
#include "Tokenize.hpp"
#include "cassert"
 
/**
void kba::scorer::RelatedEntityScorer::populateRelatedMap() {
  RDFParser labelParser;
  RDFParser relatedParser;
  using namespace boost;
  using namespace kba::entity;
  typedef unsigned char uchar;

  try {
    std::string labelKey = "labels";
    std::string relatedLabel = "internalentity";
    labelParser.initRDFParser(labelKey, _repoMap.at(labelKey));
    relatedParser.initRDFParser(relatedLabel, _repoMap.at(relatedLabel));
    if(!labelParser.getModel() || !relatedParser.getModel())
      throw std::out_of_range("RDF models not created");
  } catch(const std::out_of_range& orerror) {
    std::cout << "RelatedEntityScorer: method populateRelatedMap: failed get Repository location for labels or internalentity: "<< orerror.what() << "\n";
    return;
  }
     
  RDFQuery labelQuery(labelParser.getModel(), labelParser.getWorld());
  RDFQuery entityQuery(relatedParser.getModel(), relatedParser.getWorld()); 
  const uchar* relatedPredicate = (const unsigned char*)"http://dbpedia.org/ontology/wikiPageWikiLink";
  const uchar* labelPredicate  = (const unsigned char*)"http://www.w3.org/2000/01/rdf-schema#label";

  for (std::vector<kba::entity::Entity*>::iterator entIt = _entitySet.begin(); entIt != _entitySet.end(); ++entIt) {
    Entity* entity = *entIt;
    std::vector<shared_ptr<Entity> > relatedEntities;
    if((entity->labelTokens).size() <= 0 ) {
      std::vector<std::string> tokens = Tokenize::tokenize(entity->label);
      tokens = Tokenize::filterShortWords(tokens);
      tokens = Tokenize::toLower(tokens);
      entity->labelTokens = tokens;
    }
    
    try {
      const uchar* source = (entity->dbpediaURLs).at(0).get();
      std::vector<shared_ptr<unsigned char> > targets = entityQuery.getTargetNodes(source, relatedPredicate);
      
      for(size_t index=0; index < targets.size(); index++) {
	std::string node((const char*)(targets[index].get()));
	
	if(node.find("http://dbpedia.org/resource/Category:") != std::string::npos)  {
          continue; 
	}

        std::vector<shared_ptr<uchar> > labels = labelQuery.getTargetNodes((const uchar*)(node.c_str()), labelPredicate);   
                
        if(labels.size() > 0) {
          std::string label((const char*)(labels.at(0).get()));
	  
          shared_ptr<Entity> rEntity(new Entity());
          kba::entity::Entity* entPtr = rEntity.get();
	  //          std::string targetDbURL(node);
          entPtr->mainDbURL = node;
          relatedEntities.push_back(rEntity);     
          entPtr->label = label;
          std::vector<std::string> tokens = Tokenize::tokenize(label);
          tokens = Tokenize::filterShortWords(tokens);
          tokens = Tokenize::toLower(tokens);
          entPtr->labelTokens = tokens;
	}    
      }
    } catch(const std::out_of_range& orerror) {
      std::cout << "RelatedEntityScorer : populateRelatedMap " << orerror.what() << "\n";
    }
    _relatedMap.insert(std::pair<std::string, std::vector<boost::shared_ptr<kba::entity::Entity> > >(entity->wikiURL, relatedEntities));
  }

}
*/
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
  //  std::cout << score << "\n"; 
  return score > (float)maxScore  ? (float)maxScore : score;
}

std::vector<kba::entity::Entity*> kba::scorer::RelatedEntityScorer::getEntityList() {
  return _entitySet;
}

kba::scorer::RelatedEntityScorer::RelatedEntityScorer(std::vector<kba::entity::Entity*> entitySet, std::map<std::string, std::string> repoMap, int maxScore) : _repoMap(repoMap), _entitySet(entitySet), _maxScore(maxScore){

}
