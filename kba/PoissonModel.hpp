#ifndef POISSONMODEL_HPP
#define POISSONMODEL_HPP

#include "Model.hpp"
#include "Passage.hpp"
/**
 *Refer to paper by Mei, Fang and Zhai, A study of Poisson Query Generation Model for information retrieval
 *
 */
class PoissonModel : public Model {
private:
  float _mu;
  unsigned long _collectionSize;
  std::map<std::string, unsigned long> _collectionFreq;
public:
  PoissonModel();
  PoissonModel(float mu);
  void setCollectionSize(unsigned long collectionSize);
  void setCollectionFreq(std::string term, unsigned long freq);
  float score(std::vector<std::string>& query, Passage* psg);
  float score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize);
};
#endif
