#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "WikiEntity.hpp"

namespace kba {
  namespace scorer {
    class Scorer {
    private:
      std::vector<Entity*> _entityList;
    public:
      virtual int score(streamcorpus::StreamItem* streamItem, int maxScore)=0;
    };
  }
}
