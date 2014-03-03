#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP
#include <string>

class Distribution {
private:
public:
  virtual double termProb(std::string term, double collectionProb) = 0;
  virtual double logTermProb(std::string term, double collectionProb)= 0;
  virtual double termFreq(std::string term) = 0;
  virtual double collectionProb(std::string term) = 0;
  virtual ~Distribution() {}
};
#endif
