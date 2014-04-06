#ifndef KLDIVERGENCE_HPP
#define KLDIVERGENCE_HPP
#include "Scorer.hpp"
#include "WikiEntity.hpp"
#include "ParsedStream.hpp"
#include "TermDict.hpp"
#include <stdexcept>
namespace kba {
  namespace scorer {
    class KLDivergence : public Scorer {
    private:
      std::vector<kba::entity::Entity*> _entitySet;
      kba::term::CorpusStat*  _crpStat;
      std::map<std::string, kba::term::TermStat*> _trmStatMap;
      std::map<std::string, float> _maxScores;
      std::map<std::string, float> _collFreqMap;
      std::map<std::string, float> _logFactorCache;
      static float _mu;
      
    public:
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      void computeCollectionProb();
      void computeMaxDocScore();

      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      float getMaxScore(streamcorpus::StreamItem* streamItem, kba::entity::Entity* entity);
      void comoutMaxDocScore();
      KLDivergence(std::vector<kba::entity::Entity*> entityList, kba::term::CorpusStat* crpStat, std::map<std::string, kba::term::TermStat*> trmStatMap);
    };
  }
}
inline int kba::scorer::KLDivergence::score(streamcorpus::StreamItem* sItem, kba::entity::Entity* entity, int maxScore) {return -1;};
inline std::vector<kba::entity::Entity*> kba::scorer::KLDivergence::getEntityList() { return _entitySet;}
inline std::string kba::scorer::KLDivergence::getModelName() {return "KLDivergence";}
inline float kba::scorer::KLDivergence::getMaxScore(streamcorpus::StreamItem* streamItem, kba::entity::Entity* entity) {
  try  {
    return _maxScores[entity->wikiURL];
  } 
  catch(const std::out_of_range& expt) {
    return 0;
  }
}
#endif
