#ifndef SCORER_HPP
#define SCORER_HPP
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "WikiEntity.hpp"
#include "ParsedStream.hpp"
namespace kba {
  namespace scorer {
    class Scorer {
    public:
      virtual std::vector<kba::entity::Entity*> getEntityList() = 0;
      virtual int score(streamcorpus::StreamItem* streamItem,kba::entity::Entity* entity, int maxScore)=0;
      virtual int score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore)=0;    
    };
  }
}
#endif
