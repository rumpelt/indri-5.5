#ifndef BM25SCOREREXT_HPP
#define BM25SCOREREXT_HPP
#include "Scorer.hpp"
#include "ParsedStream.hpp"
#include "WikiEntity.hpp"
#include "TermDict.hpp"
#include "boost/shared_ptr.hpp"
#include <cmath>
#include <map>
#include <vector>
#include <string>

namespace kba {
  namespace scorer {
    class BM25ScorerExt : public Scorer{
    private:
      /**
      * The entity set for which we are working on
      */
      std::vector<kba::entity::Entity*> _entitySet;
  
     /**
       * map from wikiURL to Entities found on that wiki url. We exclude the entities of type       * "http://dbpedia.org/resource/Category:
      */
      std::map<std::string, std::vector<boost::shared_ptr<kba::entity::Entity> > > _relatedMap;
      kba::term::CorpusStat* _crpStat;
      std::map<std::string, kba::term::TermStat*> _trmStatMap;
      
      std::map<std::string, float> _idf;
      std::map<std::string, float> _maxScores; // Map from wiki url to maximum document score
      float _k1b;
      float _k1minusB;      
      
      /**
       * The maximum score which  can be assigned.
       */
      int _maxScore;
      float _parameterK;
      float _parameterB;
      float _cutoffScore;

    public:
      void computeLogIDF();
      void computeMaxDocScores();

      float computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, float maxDocScore);
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      BM25ScorerExt(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::map<std::string, kba::term::TermStat*> trmStatMap, float cutOffScore = 0);

    };
  }
}

inline std::vector<kba::entity::Entity* > kba::scorer::BM25ScorerExt::getEntityList() { return BM25ScorerExt::_entitySet;}
inline std::string kba::scorer::BM25ScorerExt::getModelName() {return "BM25Ext";}
inline int kba::scorer::BM25ScorerExt::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {return -1;}
#endif
