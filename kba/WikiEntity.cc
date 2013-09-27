#include "WikiEntity.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "cassert"

void populateEntityList(std::vector<Entity*>& entityList, std::string fileName) {
  std::ifstream entityFile(fileName.c_str());
  if(entityFile.is_open()) {
    std::string line;
    bool definationStart = false;
    Entity* entity = 0;
    while(getline(entityFile, line)) {
      boost::algorithm::trim(line);
      if(line[0] == '{') {
        definationStart = true;
        entity  = (Entity*)malloc(sizeof(Entity));
        continue;
      }
      else if(line[0] == '}') {
        if(entity != 0)
          entityList.push_back(entity);
        entity = 0; //set it null now    
        definationStart  = false;
        continue;
      }
      if (definationStart) {
        size_t index = line.find(':');
	std::string key = line.substr(0,index);
	boost::algorithm::trim(key);
	std::string value = line.substr(index+1);
	boost::algorithm::trim(value);
        if(key.compare((const char*)"\"entity_type\"")) {
          entity->entityType = value.substr(1,value.size() -1);
	}
        else if(key.compare((const char*)"\"group\"")) {
          entity->group = value.substr(1,value.size() -1);
	}
        else if(key.compare((const char*)"\"target_id\"")) {
          entity->wikiURL = value.substr(1,value.size() -1);
	}
        else {
          free(entity);
          entity=0;
          definationStart = false;
	}
      }
    }
    assert(entity == 0);
    entityFile.close();
  }
}
