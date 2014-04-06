#ifndef LANGUAGEMODELPSG_HPP
#define LANGUAGEMODELPSG_HPP
#include "Model.hpp"
#include "Passage.hpp"
#include "TermDict.hpp"
#include <map>
using namespace kba::term;
class LanguageModelPsg : public Model{
private:
  float _mu;
  unsigned long _collectionSize;
  std::map<std::string, unsigned long> _collectionFreq;

public:
  LanguageModelPsg();
  LanguageModelPsg(float mu);
  void setCollectionSize(unsigned long collectionSize);
  void setCollectionFreq(std::string term, unsigned long freq);
  float score(std::vector<std::string>& query, Passage* psg);
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize);
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string,TermStat*>& termStatMap, CorpusStat* corpusStat); 
};
#endif
