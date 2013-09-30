#include "BaseLineScorer.hpp"
                             
int kba::scorer::BaseLineScorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {
  return 600;
}

std::vector<kba::entity::Entity*> kba::scorer::BaseLineScorer::getEntityList() {
  return BaseLineScorer::_entityset;
}

kba::scorer::BaseLineScorer::BaseLineScorer(std::vector<kba::entity::Entity*> entityset) : _entityset(entityset) {
} 

