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



void kba::entity::updateEntityWithAbstract(std::vector<kba::entity::Entity*>& entityList, std::string storageDir, std::string repoName, std::unordered_set<std::string> stopSet) {
  RDFParser* rdfparser = new RDFParser();
  rdfparser->initRDFParser(repoName, storageDir);
  RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld()); 
  //  std::cout << " Abstract " << "\n";
  for(std::vector<kba::entity::Entity*>::iterator entityIt = entityList.begin(); entityIt != entityList.end(); entityIt++) {
    kba::entity::Entity* entity = *entityIt;
    if((entity->dbpediaURLs).size() > 0) {

      const unsigned char* subject = (entity->dbpediaURLs).at(0).get();
      const unsigned char* predicate = (const unsigned char*)"http://dbpedia.org/ontology/abstract";
      std::vector<boost::shared_ptr<unsigned char> > dbResourceList = rdfquery->getTargetNodes(subject, predicate);
      if (dbResourceList.size() <= 0) {
//        std::cout<< "Could not find abstract for : " <<entity->wikiURL << "\n";
      } 
      else {
        std::string abstract = (const char*)(dbResourceList.at(0).get());
        std::vector<std::string> tokens = Tokenize::tokenize(abstract, true, stopSet);
	//	std::cout << entity->wikiURL << "\n";
        for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
	  std::string tok = *tokIt;
          (entity->abstractTokens).push_back(tok);      
          (entity->textFreq)[tok]++;
	}
      }
    }
  }
  delete rdfquery;
  delete rdfparser;
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
      //      std::cout <<"\nCould not find the dbpedia resource for : "<< entity->wikiURL << "\n";
      
    }
    else {
      //      std::cout << "Adding Entity " << entity->wikiURL  << "\n";
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
    if((entity->dbpediaURLs).size() <= 0) {
      std::string wikiURL = "http://en.wikipedia.org/wiki/";
      if((entity->wikiURL).find(wikiURL) != std::string::npos) {
        entity->label = (entity->wikiURL).substr(wikiURL.size());
	//        std::cout << "Set the label from wikiurl to : " << entity->label <<  ":" <<entity->wikiURL <<"\n";
      }
      continue;
    }
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

std::vector<boost::shared_ptr<kba::entity::Entity> > kba::entity::getRelatedEntities(kba::entity::Entity* entity, std::map<std::string, std::string> repoMap, std::unordered_set<std::string> stopSet) {
  
  RDFParser labelParser;
  RDFParser relatedParser;
  using namespace boost;
  using namespace kba::entity;
  typedef unsigned char uchar;
  std::vector<shared_ptr<Entity> > relatedEntities;
  try {
    std::string labelKey = "labels";
    std::string relatedLabel = "internalentity";
    labelParser.initRDFParser(labelKey, repoMap.at(labelKey));
    relatedParser.initRDFParser(relatedLabel, repoMap.at(relatedLabel));
    if(!labelParser.getModel() || !relatedParser.getModel())
      throw std::out_of_range("RDF models not created");
  } catch(const std::out_of_range& orerror) {
    std::cout << "RelatedEntityScorer: method populateRelatedMap: failed get Repository location for labels or internalentity: "<< orerror.what() << "\n";
    return relatedEntities;
  }
     
  RDFQuery labelQuery(labelParser.getModel(), labelParser.getWorld());
  RDFQuery entityQuery(relatedParser.getModel(), relatedParser.getWorld()); 
  const uchar* relatedPredicate = (const unsigned char*)"http://dbpedia.org/ontology/wikiPageWikiLink";
  const uchar* labelPredicate  = (const unsigned char*)"http://www.w3.org/2000/01/rdf-schema#label";

  
  try {
    const uchar* source = (entity->dbpediaURLs).at(0).get();
    std::vector<shared_ptr<unsigned char> > targets = entityQuery.getTargetNodes(source, relatedPredicate);
    for(std::vector<shared_ptr<unsigned char> >::iterator targetIt = targets.begin(); targetIt != targets.end(); ++targetIt) {
      shared_ptr<unsigned char> target = *targetIt;
      std::string node((const char*)(target.get()));
	
      if(node.find("http://dbpedia.org/resource/Category:") != std::string::npos)  {
        continue; 
      }

      std::vector<shared_ptr<uchar> > labels = labelQuery.getTargetNodes((const uchar*)(node.c_str()), labelPredicate);   
                
      if(labels.size() > 0) {
        std::string label((const char*)(labels.at(0).get()));
	  
        shared_ptr<Entity> rEntity(new Entity());
        kba::entity::Entity* entPtr = rEntity.get();
	  //          std::string targetDbURL(node);
        entPtr->mainDbURL = node;
        relatedEntities.push_back(rEntity);     
        entPtr->label = label;
        std::vector<std::string> tokens = Tokenize::tokenize(label, true, stopSet);
        entPtr->labelTokens = tokens;
        for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
	  std::string tok = *tokIt;
          (entPtr->labelMap)[tok]++;      
        }
      
      }    
    }
  } catch(const std::out_of_range& orerror){
  
  }
  return relatedEntities;
}

void kba::entity::populateEntityStruct(std::vector<kba::entity::Entity*>& entityList, std::map<std::string, std::string> repoMap, std::unordered_set<std::string> stopSet) {
  try {
    kba::entity::updateEntityWithDbpedia(entityList, repoMap.at("wikiToDb"), "wikiToDb" );
    kba::entity::updateEntityWithLabels(entityList, repoMap.at("labels"), "labels" );
    for(std::vector<kba::entity::Entity*>::iterator entIt = entityList.begin(); entIt != entityList.end(); entIt++) {
      kba::entity::Entity* entity = *entIt;
      std::vector<std::string> tokens = Tokenize::tokenize(entity->label, true, stopSet);
      entity->labelTokens = tokens;
      for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
	std::string tok = *tokIt;
        (entity->labelMap)[tok]++;      
      }
      entity->relatedEntities = kba::entity::getRelatedEntities(entity, repoMap, stopSet);
    }
  } catch (const std::out_of_range& oor) {
  }
}


