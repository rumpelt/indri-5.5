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
#include <stdexcept>
using namespace boost;
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
      std::map<std::string, kba::term::TermStat*> _trmStatMap;
      
      std::map<std::string, float> _idf;
      std::map<std::string, float> _maxScores; // Map from wiki url to maximum document score
      float _k1b;
      float _k1minusB;      

      float _cutoffScore;
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
      float getMaxScore(streamcorpus::StreamItem* streamItem, kba::entity::Entity* entity);
      BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::map<std::string, kba::term::TermStat*> trmStatMap, float _cutoffScore, int maxScore=1000);

    };
  }
}

inline std::vector<kba::entity::Entity* > kba::scorer::BM25Scorer::getEntityList() { return BM25Scorer::_entitySet;}
inline std::string kba::scorer::BM25Scorer::getModelName() {return "BM25";}
inline int kba::scorer::BM25Scorer::score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore) {return -1;}

inline float kba::scorer::BM25Scorer::getMaxScore(streamcorpus::StreamItem* streamItem, kba::entity::Entity* entity) {
  try  {
    return _maxScores[entity->wikiURL];
  } 
  catch(const std::out_of_range& expt) {
    return 0;
  }
}

inline float kba::scorer::BM25Scorer::score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore) {
  
  using namespace boost;
  using namespace kba::entity;
  float cutoff = 3.0; // The minimu  third quartile observed
  float score = kba::scorer::BM25Scorer::computeNormalizedDocScore(parsedStream, entity->labelTokens, 0);
  return score > cutoff ? score : 0;   
}

#endif
