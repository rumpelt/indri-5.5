#ifndef BM25PSG_HPP
#define BM25PSG_HPP
#include "Model.hpp"
#include "Passage.hpp"
#include "QueryThread.hpp"

class BM25Psg : public Model {
private:
  unsigned long _collectionSize;
  std::map<std::string, unsigned long> _collectionFreq;
public:
  void setCollectionSize(unsigned long collectionSize);
  void setCollectionFreq(std::string term, unsigned long freq);
  float score(std::vector<std::string>& query, Passage* psg);
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize);

};
#endif
