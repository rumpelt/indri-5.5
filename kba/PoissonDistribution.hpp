#ifndef POISSONDISTRIBUTION_HPP
#define POISSONDISTRIBUTION_HPP

#include "Distribution.hpp"
#include <map>

class PoissonDistribution : public Distribution {
private:
  double _mu; // set set _mu to zero for no smoothing
  double _size; //this should be document size
  std::map<std::string, unsigned long> _freqMap;
  std::map<std::string, double> _collectionProb;
  std::map<std::string, double> _lambdaEstimate;
public:
  PoissonDistribution(double mu);
  PoissonDistribution();
  ~PoissonDistribution();   
  void initialize(std::map<std::string, unsigned long>& freqMap, std::map<std::string, double>& collectionProb, unsigned long docSize);
  double termProb(std::string term, double collectionProb); // This can suffer from interger overflow problem due to factorial computation which very easily overruns. In cse of over flow we just take maximum value of unsigned long ...look at implementation.
  double logTermProb(std::string term, double collectionProb);
  double termFreq(std::string term);
  double collectionProb(std::string term);
};
#endif
