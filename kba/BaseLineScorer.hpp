#include "Scorer.hpp"
namespace kba
{
  namespace scorer
  {
    class BaseLineScorer : Scorer {
      int score(streamcorpus::StreamItem* stream, Entity* entity, int maxScore);
    };
  }
}
