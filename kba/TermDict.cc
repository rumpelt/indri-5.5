#include "TermDict.hpp"
#include "stdexcept"
#include <boost/shared_ptr.hpp>

float& kba::term::LOG_OF_2() { static float logOf2  = log(2.0); return logOf2;}


void kba::term::updateTermBase(kba::stream::ParsedStream* parsedStream, kba::term::TermBase* termBase) {
  size_t docSize= (parsedStream->tokens).size();
  
  if( docSize > 0) {
    termBase->totalDocs = termBase->totalDocs + 1;
    termBase->totalDocLength = termBase->totalDocLength +  docSize;
    termBase->avgDocLength = (1.0 * termBase->totalDocLength) / termBase->totalDocs;
  }

  for(std::set<std::string>::iterator termIt = (termBase->vocabulary).begin(); termIt != (termBase->vocabulary).end(); ++termIt) {
    std::string term = *termIt;
    unsigned int value = 1;
    if((parsedStream->tokenSet).count(term) > 0) {
      try {
	value  = (termBase->termDocFreq).at(term);
        value = value + 1;
        (termBase->termDocFreq).erase(term);
        (termBase->termDocFreq).insert(std::pair<std::string, unsigned int>(term, value));
         
      } catch(std::out_of_range& oor) {
        (termBase->termDocFreq).insert(std::pair<std::string, unsigned int>(term, value));
      }
      
      float idf = log((termBase->totalDocs - value + 0.5 )/ (value + 0.5)) ;
      idf = idf/kba::term::LOG2;
      if(idf < 0.0) // In case IDF is less than zero..This can happe
        idf = 0.0;
      (termBase->logIDF).erase(term);
      (termBase->logIDF).insert(std::pair<std::string, float> (term, idf));
      
    } 
  }
}


void kba::term::populateVocabulary(std::vector<kba::entity::Entity*> entityList, kba::term::TermBase* termBase) {

  using namespace kba::entity;
  using namespace boost;
  for(std::vector<Entity*>::iterator entIt = entityList.begin(); entIt != entityList.end(); ++entIt) {
    Entity* entity = *entIt;
    for(std::vector<std::string>::iterator tokenIt = (entity->labelTokens).begin(); tokenIt != (entity->labelTokens).end(); ++tokenIt) {
      std::string token = *tokenIt;
      (termBase->vocabulary).insert(token);
    }
    
    for (std::vector<shared_ptr<Entity> >::iterator relEntIt = (entity->relatedEntities).begin();   relEntIt != (entity->relatedEntities).end(); ++relEntIt) {
      Entity* related = (*relEntIt).get();
      for(std::vector<std::string>::iterator tokenIt = (related->labelTokens).begin(); tokenIt != (related->labelTokens).end(); ++tokenIt) {
        std::string token = *tokenIt;
        (termBase->vocabulary).insert(token);
      } 
    }
  }
}

kba::term::TermBase::TermBase(std::vector<kba::entity::Entity*> entityList) : totalDocLength(0), totalDocs(0), avgDocLength(0.0) {
  kba::term::populateVocabulary(entityList, this);
} 

std::set<kba::term::TopicStat*> kba::term::topicStatSet(std::vector<kba::entity::Entity*> entitySet) {
  using namespace kba::entity;

  std::set<kba::term::TopicStat*> topicStatSet;

  for(std::vector<Entity*>::iterator entIt = entitySet.begin(); entIt != entitySet.end(); ++entIt) {
    TopicStat* topicStat = new TopicStat();
    topicStat->topic = (*entIt)->wikiURL;
    if(topicStatSet.find(topicStat) == topicStatSet.end())
      topicStatSet.insert(topicStat);
    else {
      delete topicStat;
    } 
  }
  return topicStatSet;
}

std::set<kba::term::TermStat*> kba::term::termStatSet(std::vector<kba::entity::Entity*> entitySet) {
  using namespace kba::entity;
  using namespace std;
  std::set<kba::term::TermStat*> termStatSet;

  for(std::vector<Entity*>::iterator entIt = entitySet.begin(); entIt != entitySet.end(); ++entIt) {
     Entity* entity = *entIt;
    for(set<string>::iterator tokIt = (entity->tokenSet).begin(); tokIt != (entity->tokenSet).end(); ++tokIt) {
      TermStat* termStat = new TermStat();
      termStat->term = *tokIt;
      if(termStatSet.find(termStat) == termStatSet.end())
        termStatSet.insert(termStat);
      else
        delete termStat;
    } 

    for(vector<boost::shared_ptr<Entity> >::iterator relIt = (entity->relatedEntities).begin(); relIt != (entity->relatedEntities).end(); ++relIt) {
      Entity* ent = (*relIt).get();
      for(set<string>::iterator tokIt = (ent->tokenSet).begin(); tokIt != (ent->tokenSet).end(); ++tokIt) {
        TermStat* termStat = new TermStat();
        termStat->term = *tokIt;
        if(termStatSet.find(termStat) == termStatSet.end())
          termStatSet.insert(termStat);
        else
          delete termStat;
        
      } 
    }
  }
  return termStatSet;
}

std::map<kba::term::TopicTermKey*, kba::term::TopicTermValue*> kba::term::topicTermMap(std::vector<kba::entity::Entity*> entitySet) {
  using namespace kba::entity;
  using namespace kba::term;
  using namespace std;
  std::map<TopicTermKey*, TopicTermValue*> topicTermMap;

  for(std::vector<Entity*>::iterator entIt = entitySet.begin(); entIt != entitySet.end(); ++entIt) {
     Entity* entity = *entIt;
    for(set<string>::iterator tokIt = (entity->tokenSet).begin(); tokIt != (entity->tokenSet).end(); ++tokIt) {
      TopicTermKey* termKey = new TopicTermKey();
      termKey->topic = entity->wikiURL;
      termKey->term = *tokIt;
      TopicTermValue* termValue = new TopicTermValue();
      if(topicTermMap.find(termKey) == topicTermMap.end()) {
        topicTermMap.insert(std::pair<TopicTermKey*,TopicTermValue*>(termKey, termValue));
      }
      else {
        delete termKey;
        delete termValue;
      }
    } 

    for(vector<boost::shared_ptr<Entity> >::iterator relIt = (entity->relatedEntities).begin(); relIt != (entity->relatedEntities).end(); ++relIt) {
      Entity* ent = (*relIt).get();
      for(set<string>::iterator tokIt = (ent->tokenSet).begin(); tokIt != (ent->tokenSet).end(); ++tokIt) {
         TopicTermKey* termKey = new TopicTermKey();
         termKey->topic = ent->wikiURL;
         termKey->term = *tokIt;
         TopicTermValue* termValue = new TopicTermValue();
         if(topicTermMap.find(termKey) == topicTermMap.end()) {
           topicTermMap.insert(std::pair<TopicTermKey*,TopicTermValue*>(termKey, termValue));
         }
         else {
           delete termKey;
           delete termValue;
         } 
       } 
    }
  } 
 
  return topicTermMap;
}
