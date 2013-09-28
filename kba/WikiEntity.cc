#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include "WikiEntity.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "cassert"
#include "Tokenize.hpp"

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




void updateEntityWithDbpedia(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName) {
  RDFParser* rdfparser = new RDFParser(storageDir, repoName, std::string("ntriples"),std::string( "bdb"), false);
  RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld()); 
  for(std::vector<Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    Entity* entity = *entityIt;
    const unsigned char* subject = (const unsigned char*)((entity->wikiURL).c_str());
    const unsigned char* predicate = (const unsigned char*)"http://xmlns.com/foaf/0.1/primaryTopic";
    std::vector<boost::shared_ptr<unsigned char> > dbResourceList = rdfquery->getTargetNodes(subject, predicate);
    if(dbResourceList.size() <= 0) {
      std::cout <<"\nCould not find the dbpedia resource for : "<< entity->wikiURL << "\n";
    }
    else {
      entity->dbpediaURLs = dbResourceList;
    }
  }
  
  free(rdfquery);
  free(rdfparser);
}

void updateEntityWithLabels(std::vector<Entity*>& entityList, std::string storageDir, std::string repoName) {
  RDFParser* rdfparser = new RDFParser(storageDir, repoName, std::string("ntriples"),std::string( "bdb"), false);
  RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld()); 
  for(std::vector<Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    Entity* entity = *entityIt;
    const unsigned char* predicate  = (const unsigned char*)"http://www.w3.org/2000/01/rdf-schema#label";
    for(std::vector<boost::shared_ptr<unsigned char> >::iterator urlIt = (entity->dbpediaURLs).begin(); urlIt != (entity->dbpediaURLs).end(); urlIt++) {
      boost::shared_ptr<unsigned char>  url = *urlIt;
      const unsigned char* subject = (const unsigned char*)url.get();
      std::vector<boost::shared_ptr<unsigned char> > labelList = rdfquery->getTargetNodes(subject, predicate);
      if (labelList.size() <= 0) {
	std::cout << "Could not find the label for : "<< entity->wikiURL <<"\n"; 
      }
      else if (labelList.size() > 1){
        std::cout << "Too many labels for : "<< entity->wikiURL <<"\n"; 
      }
      else {
	boost::shared_ptr<unsigned char> label = labelList[0];
	std::string inputSource = (const char*)label.get(); 
	std::vector<std::string> tokens = Tokenize::tokenize(inputSource);
        tokens = Tokenize::toLower(tokens);
        for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
	  (entity->unigrams).push_back(*tokIt);
	}
      }
    }
  }
  free(rdfquery);
  free(rdfparser);
  
}
