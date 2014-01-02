#ifndef LANGUAGEMODELPSG_HPP
#define LANGUAGEMODELPSG_HPP

#include "Passage.hpp"
#include "TermDict.hpp"
#include <map>
using namespace kba::term;
class LanguageModelPsg {
private:
  float _mu;
public:
  LanguageModelPsg();
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize);
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string,TermStat*>& termStatMap, CorpusStat* corpusStat); 
};
#endif
