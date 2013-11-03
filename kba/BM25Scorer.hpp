#ifndef BM25SCORER_HPP
#define BM25SCORER_HPP

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
    class BM25Scorer : public Scorer{
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
      std::set<kba::term::TermStat*> _trmStat;
      
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
     
    public:
      void computeLogIDF();
      void computeMaxDocScores();

      float computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, float maxDocScore);
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::set<kba::term::TermStat*> trmStat, int maxScore=1000);
    };
  }
}

inline std::vector<kba::entity::Entity* > kba::scorer::BM25Scorer::getEntityList() { return BM25Scorer::_entitySet;}
inline std::string getModelName() {return "BM25-Normalized";}

#endif
