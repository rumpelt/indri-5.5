#ifndef BASELINESCORER_HPP
#define BASELINESCORER_HPP
#include "Scorer.hpp"

//#include <vector>
namespace kba
{
  namespace scorer
  {
    class BaseLineScorer : public Scorer {
    private:
      std::vector<kba::entity::Entity* > _entityset;    
    public:
      std::vector<kba::entity::Entity* > getEntityList();
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      int score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);  
      BaseLineScorer(std::vector<kba::entity::Entity*> entityset);
    };
  }
}
#endif
