#ifndef BM25RSJSCORER_HPP
#define BM25RSJSCORER_HPP

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
    class BM25RSJScorer : public Scorer{
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
      std::set<kba::term::TopicTerm*> _tpcTrm;
      std::map<std::string, float> _rsj;
      std::map<std::string, float> _maxScores; // Map from wiki url to maximum document score
      float _k1b;
      float _k1minusB;      
      //     time_t _startStamp;
      /**
       * The maximum score which  can be assigned.
       */
      int _maxScore;
      float _parameterK;
      float _parameterB;
     
    public:
      void computeLogRSJWeight();
      void computeMaxDocScores();

      float computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms, std::string entity, float maxDocScore);
      std::string getModelName();
      std::vector<kba::entity::Entity* > getEntityList(); 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      BM25RSJScorer(std::vector<kba::entity::Entity*> entitySet,  kba::term::CorpusStat* crpStat, std::set<kba::term::TermStat*> trmStat, std::set<kba::term::TopicTerm*> tpcTrm, int maxScore=1000);
    };
  }
}

inline std::vector<kba::entity::Entity* > kba::scorer::BM25RSJScorer::getEntityList() { return BM25RSJScorer::_entitySet;}
inline std::string kba::scorer::BM25RSJScorer::getModelName() {return "BM25RSJ";}

#endif
