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
   
      boost::shared_ptr<kba::term::TermBase> _termBase;
      
      
      /**
       * The maximum score which  can be assigned.
       */
      int _maxScore;
      float _parameterK;
      float _parameterB;
     
    public:
      float computeQueryTermIDF(std::string term);
      float computeNormalizedDocScore(kba::stream::ParsedStream* stream, std::vector<std::string> queryTerms);


      std::vector<kba::entity::Entity* > getEntityList();
 
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int maxScore);
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int maxScore);
      BM25Scorer(std::vector<kba::entity::Entity*> entitySet,  boost::shared_ptr<kba::term:: TermBase>  termBase, int maxScore=1000);
    };
  }
}

inline std::vector<kba::entity::Entity* > kba::scorer::BM25Scorer::getEntityList() { return BM25Scorer::_entitySet};

#endif
