#include "Model.hpp"

void Model::setCollectionSize(unsigned long collSize) {
}

void Model::setCollectionFreq(std::string, unsigned long freq) {
}

float Model::score(query_t& query, Distribution& psg) {
  return 0;
}

float Model::score(std::vector<std::string>& query, Passage* psg) {
  //     std::cout << "worong " << std::endl;
  return 0;
}
float Model::score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize) {
  return 0;
}

float Model::score(std::vector<std::string>& query, Passage* psg, std::map<std::string,kba::term::TermStat*>& termStatMap, kba::term::CorpusStat* corpusStat) {
  return 0;
}
