#include <string>
#include <vector>
#include <map>
#include <fstream>

struct Entity {
  std::vector<std::string> unigrams;
  std::string wikiURL;
  std::string entityType; // may be later conver to enum
  std::string group; // as defined in topic file released by kba
  std::string dbpediaURL;
};

/**
 * the kba entity map.
 */
std::map<std::string, std::vector<Entity*> > KbaEntityMap;
void populateEntityMap(std::map<std::string, std::vector<Entity*> >& entityMap, std::string fileName);
void populateEntityList(std::vector<Entity*>& entityList, std::string fileName);
