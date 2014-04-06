#ifndef MULTINOMIALDISTRIBUTION
#define MULTINOMIALDISTRIBUTION
#include <string>
#include <map>
#include "Distribution.hpp"

class MultinomialDistribution : public Distribution {
private:
  std::map<std::string, unsigned long> _termFreq;
  std::map<std::string, double> _collProb;
  double _collectionSize;
  double _docSize;
  double _mu;
public:
  MultinomialDistribution(double _mu);
  ~MultinomialDistribution();   
   
  /**
   * Must be called after creation to set the maps  properly
   */
  void initialize(std::map<std::string, unsigned long>& freqMap, std::map<std::string, double>& collectionProb, unsigned long docSize); 

  double termProb(std::string term, double collectionProb); // This can suffer from interger overflow problem due to factorial computation which very easily overruns. In cse of over flow we just take maximum value of unsigned long ...look at implementation.
  double logTermProb(std::string term, double collectionProb);
  double termFreq(std::string term);
  double collectionProb(std::string term);

};
#endif
