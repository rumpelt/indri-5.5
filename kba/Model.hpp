#ifndef MODEL_HPP
#define MODEL_HPP

#include "TermDict.hpp"
#include "Passage.hpp"
#include "Distribution.hpp"
#include "QueryThread.hpp" 
class Model {
private:
public:
  virtual void  setCollectionSize(unsigned long collSize);
  virtual void setCollectionFreq(std::string term, unsigned long freq);

  virtual float score(query_t& query, Distribution& psg);

  virtual float score(std::vector<std::string>& query, Passage* psg);
  virtual float score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize);
  virtual float score(std::vector<std::string>& query, Passage* psg, std::map<std::string,kba::term::TermStat*>& termStatMap, kba::term::CorpusStat* corpusStat);    
};
#endif
