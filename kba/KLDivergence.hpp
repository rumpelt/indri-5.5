#ifndef KLDIVERGENCE_HPP
#define KLDIVERGENCE_HPP
#include "Scorer.hpp"
#include "WikiEntity.hpp"
#include "ParsedStream.hpp"
#include "TermDict.hpp"

namespace kba {
  namespace scorer {
    class KLDivergence : public Scorer {
    private:
      std::vector<kba::entity::Entity*> _entitySet;
      kba::term::CorpusStat*  _crpStat;
      std::map<std::string, kba::term::TermStat*> _trmStatMap;
      static float _alphaD;
      static float _logAlphaD;

    public:
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      KLDivergence(std::vector<kba::entity::Entity*> entityList, kba::term::CorpusStat* crpStat, std::map<std::string, kba::term::TermStat*> trmStatMap);
    };
  }
}
inline int kba::scorer::KLDivergence::score(streamcorpus::StreamItem* sItem, kba::entity::Entity* entity, int maxScore) {return -1;};
inline std::vector<kba::entity::Entity*> kba::scorer::KLDivergence::getEntityList() { return _entitySet;}
inline std::string kba::scorer::KLDivergence::getModelName() {return "KLDivergence";}
#endif
