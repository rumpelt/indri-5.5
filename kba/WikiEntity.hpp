#ifndef WIKIENTITY_HPP
#define WIKIENTITY_HPP
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include "boost/shared_ptr.hpp"

namespace kba {
  namespace entity {

  struct Entity {
    std::string label;
    std::vector<std::string> labelTokens; // tokens of the label.
    std::string wikiURL;
    std::string mainDbURL;
    std::string entityType; // may be later conver to enum
    std::string group; // as defined in topic file released by kba
    std::vector<boost::shared_ptr<unsigned char> > dbpediaURLs;
  };

 /**
  * the kba entity map.
  */
  //std::map<std::string, std::vector<Entity*> > KbaEntityMap;
  void updateEntityWithDbpedia(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName);
  void populateEntityList(std::vector<Entity*>& entityList, std::string fileName);

  void updateEntityWithLabels(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName);
  }
}
#endif
