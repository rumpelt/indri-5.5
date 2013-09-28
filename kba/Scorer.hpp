#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "WikiEntity.hpp"

namespace kba {
  namespace scorer {
    class Scorer {
    public:
      virtual int score(streamcorpus::StreamItem* streamItem, Entity* entity, int maxScore) = 0;
    };
  }
}
