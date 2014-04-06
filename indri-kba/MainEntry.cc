#include "Query.hpp"
#include <vector>

struct TruthData {
  std::string topic;
  std::string stream_id;
  int size;
  int rating;
};

void parseTruthFile(std::string& fileName, std::vector<TruthData*>& truthData) {
  std::ifstream truthFile(fileName.c_str());
  if(truthFile.is_open()) {
    std::string line;
    getline(truthFile,line);
    while(getline(truthFile, line)) {
      boost::algorithm::trim(line); 
    }
  }
  truthFile.close();
}

std::vector<Query> populateEntityList(std::string fileName) {
  std::ifstream entityFile(fileName.c_str());
  std::vector<Query> queryList;
  if(entityFile.is_open()) {
    std::string line;
    bool definationStart = false;
    Query query;
    while(getline(entityFile, line)) {
      boost::algorithm::trim(line);
      if(line[0] == '{') {
        definationStart = true;
        continue;
      }
      else if(line[0] == '}') {
        if(query.qnum.size() > 0) {
          queryList.push_back(query);
	  //	  std::cout <<  "Freeing up entuty at deination end \n";
          query.qnum.clear();
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
          query.queryType = value;
	}
        else if(!key.compare((const char*)"\"group\"")) {
	  //          std::cout << " Key : " << key << " value : " << value << " and line : "<< entity << "\n";  
          
          size_t first = value.find_first_of('\"');
          size_t last = value.find_last_of('\"');
	  value = value.substr(first+1, last - first -1);
	  query.group = value;

	}
        else if(!key.compare((const char*)"\"target_id\"")) {
          //std::cout << " Key : " << key << " value : " << value << " and line : "<< line << "\n";  
          size_t first = value.find_first_of('\"');
          size_t last = value.find_last_of('\"');
	  value = value.substr(first+1, last - first -1);
          query.qnum = value;

	}
        else {
	  //	  std::cout << "key : " << key << "\n";
          //if(entity != 0)
          query.qnum.clear();
          definationStart = false;
	}
      }
    }
    assert(entity == 0);
    entityFile.close();
  }
}

int main(int argc , char* argv[]) {
}

