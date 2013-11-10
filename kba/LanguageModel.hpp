#ifndef LANGUAGEMODEL_HPP
#define LANGUAGEMODEL_HPP
#include "Scorer.hpp"
#include "TermDict.hpp"
#include "WikiEntity.hpp"

namespace kba {
  namespace scorer {
    class LanguageModel : public Scorer {
    private:
      std::vector<kba::entity::Entity*> _entitySet;
      std::map<std::string, kba::term::TermStat*> _trmStatMap;
      kba::term::CorpusStat* _crpStat;
      
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
      LanguageModel(std::vector<kba::entity::Entity*>& entitySet, std::map<std::string, kba::term::TermStat*> trmStatMap, kba::term::CorpusStat* crpStat,float mu=2500);
    };
  }
}
inline int kba::scorer::LanguageModel::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {return -1;}

#endif
