#include "MultinomialDistribution.hpp"
#include "math.h"
#include "map"
#include "stdio.h"
#include <iostream>

MultinomialDistribution::MultinomialDistribution(double mu) : _mu(mu) {}
MultinomialDistribution::~MultinomialDistribution() {}

double MultinomialDistribution::termProb(std::string term, double collectionProb) {
  //  std::cout << "Term " << term << " freq " << _termFreq[term]  << " doc zie " << _docSize << std::endl;
  double freq = _termFreq[term] + _mu * collectionProb;
  double docSize =  _docSize + _mu;
  return  freq / docSize;
}

double MultinomialDistribution::logTermProb(std::string term, double collectionProb) {
  double freq = _termFreq[term] + _mu * collectionProb;
  double docSize = _docSize + _mu;
  return log(freq)- log(docSize);  
}

double MultinomialDistribution::termFreq(std::string term) {
  return _termFreq[term];
}

double MultinomialDistribution::collectionProb(std::string term) {
  return _collProb[term];
}


void MultinomialDistribution::initialize(std::map<std::string, unsigned long>& freqMap, std::map<std::string, double>& collectionProb, unsigned long docSize) {
  _termFreq = freqMap;
  _collProb = collectionProb;
  _docSize = docSize;
}
