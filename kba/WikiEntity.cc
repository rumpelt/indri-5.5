#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include "WikiEntity.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "cassert"
#include "Tokenize.hpp"

void kba::entity::populateEntityList(std::vector<kba::entity::Entity*>& entityList, std::string fileName) {
  std::ifstream entityFile(fileName.c_str());
  if(entityFile.is_open()) {
    std::string line;
    bool definationStart = false;
    kba::entity::Entity* entity = 0;
    while(getline(entityFile, line)) {
      boost::algorithm::trim(line);
      if(line[0] == '{') {
        definationStart = true;
        entity  = new kba::entity::Entity();
        continue;
      }
      else if(line[0] == '}') {
        if(entity != 0) {
          entityList.push_back(entity);
	  //	  std::cout <<  "Freeing up entuty at deination end \n";
          entity=0;
        }  
        definationStart  = false;
        continue;
      }
      if (definationStart) {
        size_t index = line.find(':');
	std::string key = line.substr(0,index);
	boost::algorithm::trim(key);
 
	std::string value;
        if(line.size() > index)
         value = line.substr(index+1);
	boost::algorithm::trim(value);

        if(!key.compare((const char*)"\"entity_type\"")) {
	  //std::cout << " Key : " << key << " value : " << value << " and line : "<< line << "\n";   
	  //std::cout << " Entut : " << entity << "\n";
          size_t first = value.find_first_of('\"');
          size_t last = value.find_last_of('\"');
	  value = value.substr(first+1, last - first -1);
           (*entity).entityType = value;

	}
        else if(!key.compare((const char*)"\"group\"")) {
	  //          std::cout << " Key : " << key << " value : " << value << " and line : "<< entity << "\n";  
          
          size_t first = value.find_first_of('\"');
          size_t last = value.find_last_of('\"');
	  value = value.substr(first+1, last - first -1);
	  (*entity).group = value;

	}
        else if(!key.compare((const char*)"\"target_id\"")) {
          //std::cout << " Key : " << key << " value : " << value << " and line : "<< line << "\n";  
          size_t first = value.find_first_of('\"');
          size_t last = value.find_last_of('\"');
	  value = value.substr(first+1, last - first -1);
          (*entity).wikiURL = value;

	}
        else {
	  //	  std::cout << "key : " << key << "\n";
          //if(entity != 0)
          assert(entity != 0);
          delete entity;
          entity = 0;
          definationStart = false;
	}
      }
    }
    assert(entity == 0);
    entityFile.close();
  }
}




void kba::entity::updateEntityWithDbpedia(std::vector<kba::entity::Entity*>& entityList, std::string storageDir, std::string repoName) {
  RDFParser* rdfparser = new RDFParser();
  rdfparser->initRDFParser(repoName, storageDir);
  RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld()); 
  for(std::vector<kba::entity::Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    kba::entity::Entity* entity = *entityIt;
    const unsigned char* subject = (const unsigned char*)((entity->wikiURL).c_str());
    const unsigned char* predicate = (const unsigned char*)"http://xmlns.com/foaf/0.1/primaryTopic";
    std::vector<boost::shared_ptr<unsigned char> > dbResourceList = rdfquery->getTargetNodes(subject, predicate);
    if(dbResourceList.size() <= 0) {
      std::cout <<"\nCould not find the dbpedia resource for : "<< entity->wikiURL << "\n";
    }
    else {
      if(dbResourceList.size() > 1)
	 std::cout << "More than one dbpedia resource for " << entity->wikiURL << "\n";
      entity->dbpediaURLs = dbResourceList;
    }
  }
  
  delete rdfquery;
  delete rdfparser;
}



  
void kba::entity::updateEntityWithLabels(std::vector<kba::entity::Entity*>& entityList, std::string storageDir, std::string repoName) {
  RDFParser* rdfparser = new RDFParser();
  rdfparser->initRDFParser(repoName, storageDir);
  RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld()); 
  for(std::vector<kba::entity::Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    kba::entity::Entity* entity = *entityIt;
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
	entity->label = (const char*)label.get(); 
	
      }
    }
  }
  delete rdfquery;
  delete rdfparser;
  
}
