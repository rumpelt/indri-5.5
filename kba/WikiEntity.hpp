#ifndef WIKIENTITY_HPP
#define WIKIENTITY_HPP
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include "boost/shared_ptr.hpp"
#include <unordered_set>
namespace kba {
  namespace entity {
    /**
     * While creating entity , must set as muhc as info possible.
     * But at min need to set follwing.
     * wikiURL, mainDbURL or dbpediaURLs.
     */
  struct Entity {
    std::string label;
    std::vector<std::string> labelTokens; // tokens of the label.
    std::set<std::string> tokenSet; //same as labelTokens above but the unique tokens only
    std::string wikiURL; // corresponds to wikiurl
    std::string mainDbURL; // either this one will be populatd or dbpediaURLs will be set
    std::string entityType; // may be later conver to enum, 
    std::string group; // as defined in topic file released by kba
    std::vector<boost::shared_ptr<unsigned char> > dbpediaURLs; // the list of dbpedia URI pointing to this wiki..It should be one but I have not verified.
    std::vector<boost::shared_ptr<kba::entity::Entity> > relatedEntities; 
  };
  

 /**
  * the kba entity map.
  */
  //std::map<std::string, std::vector<Entity*> > KbaEntityMap;
  void updateEntityWithDbpedia(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName);
  
  void populateEntityList(std::vector<Entity*>& entityList, std::string fileName);

  void updateEntityWithLabels(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName);  

  /**
   * get the related entities of an entity from dbpedia. Also populates the label field of these entites. Set the labelTokens and tokenSet field for each of the related entity.
  */
    std::vector<boost::shared_ptr<Entity> > getRelatedEntities(Entity* entity, std::map<std::string, std::string> repoMap, std::unordered_set<std::string> stopSet);

    void populateEntityStruct(std::vector<Entity*>& entityList, std::map<std::string, std::string> repoMap, std::unordered_set<std::string> stopSet);
  }
}
#endif
