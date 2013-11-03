#ifndef LANGUAGEMODEL_HPP
#define LANGUAGEMODEL_HPP
#include "Scorer.hpp"
#include "TermDict.hpp"
#include "WikiEntity.hpp"

namespace kba {
  namespace scorer {
    class LanguageModel : public Scorer {
    private:
      std::set<kba::entity::Entity*> _entitySet;
      kba::term::CorpusStat* _crpStat;
      std::set<kba::term::TermStat*> _trmStat;
      float _mu;
      std::map<std::string, float> _collFreqMap;
      std::map<std::string, float> _maxScoreMap;
    public:
      /**
       * calculates the collection term probabilities of terms. also takes in account the
       * _mu factor for dirichlet smoothing.
       */
 
      void computeCollectionProb(); 
      void computeMaxDocScore();
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      LanguageModel(std::set<kba::entity::Entity*>& entitySet, std::set<kba::term::TermStat*> trmStat, float mu=2500);
    };
  }
}
inline int kba::scorer::LanguageModel::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {return -1;}

#endif
