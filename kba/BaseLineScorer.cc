#include "BaseLineScorer.hpp"
#include "StreamUtils.hpp"
#include "Tokenize.hpp"
#include <cassert>
                           
int kba::scorer::BaseLineScorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {
  std::string entityLabel = entity->label;
    if (entityLabel.size() <= 0) {
    std::cout << "Not a vailid entiy " << entity->wikiURL << "\n";
    return 0;
  }

  std::string title = streamcorpus::utils::getTitle(*stream);
  if(title.size() <= 0) {
    title = streamcorpus::utils::getAnchor(*stream);
    if(title.size() < 0 )
      return 0;
  }
  
  
  std::vector<std::string> entityTokens = Tokenize::tokenize(entityLabel);
  entityTokens = Tokenize::toLower(entityTokens);
  assert(entityTokens.size() > 0);  

  std::vector<std::string> titlePhrases = Tokenize::getPhrases(title);
  titlePhrases = Tokenize::toLower(titlePhrases);
  
  int score = 0;
  int step = maxScore / entityTokens.size();
  for(std::vector<std::string>::iterator entTokIt = entityTokens.begin(); entTokIt != entityTokens.end(); entTokIt++) {  
    std::string tok = *entTokIt;
    for (std::vector<std::string>::iterator titTokIt = titlePhrases.begin(); titTokIt != titlePhrases.end(); titTokIt++) {
      std::string titTok = *titTokIt;
      if(titTok.find(tok) != std::string::npos)
        score  = score + step;
    }
  }     
 
  if(score > maxScore) 
    score = maxScore;
 
   return score;
  
}
                                 
std::vector<kba::entity::Entity*> kba::scorer::BaseLineScorer::getEntityList() {
  return BaseLineScorer::_entityset;
}

kba::scorer::BaseLineScorer::BaseLineScorer(std::vector<kba::entity::Entity*> entityset) : _entityset(entityset) {
} 

