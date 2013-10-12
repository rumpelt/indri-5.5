#ifndef RELATEDENTITYSCORER
#define RELATEDENTITYSCORER
#include "Scorer.hpp"
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "WikiEntity.hpp"


namespace kba 
{
  namespace scorer 
  {
    class RelatedEntityScorer : public Scorer {
    private:
      /**
       * map from repo name to repository
       */
      std::map<std::string, std::string> _repoMap; // Redland repositories for geting the labels of wikipedia and corresponding entities mentioned
      /**
       * map from wikiURL to Entities found on that wiki url. We exclude the entities of type     
       * "http://dbpedia.org/resource/Category:
       */
      std::map<std::string, std::vector<boost::shared_ptr<kba::entity::Entity> > > _relatedMap; 
      /**
       * The entity set for which we are working on
       */
      std::vector<kba::entity::Entity*> _entitySet;
      /**
       * The maximum score which  can be assigned.
       */
      int _maxScore;
      
      
    public:
      /**
       * Populate the map _realtedMap;
       */
      //      void populateRelatedMap();

      std::vector<kba::entity::Entity* > getEntityList();
      float score(kba::stream::ParsedStream* parsedStream, kba::entity::Entity* entity, int cutOffScore);       
      int score(streamcorpus::StreamItem* stream, kba::entity::Entity* entity, int cutOffScore);       
      RelatedEntityScorer(std::vector<kba::entity::Entity*> entitySet, std::map<std::string, std::string> repoMap, int maxScore=1000);  
    };
  }
}
#endif
