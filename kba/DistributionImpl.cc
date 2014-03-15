#include  "DistributionImpl.hpp"
#include <map>
#include <math.h>

void DistributionImpl::setTermProb(std::map<std::string, double> termProb) {
  DistributionImpl::_termProb = termProb;
}

void DistributionImpl::setTermFreq(std::map<std::string, double> termFreq) {
  DistributionImpl::_termFreq = termFreq;
}

void DistributionImpl::setCollectionProb(std::map<std::string, double> collProb) {
  DistributionImpl::_collProb = collProb;
}

double DistributionImpl::termProb(std::string term, double collectionProb) {
  return DistributionImpl::_termProb[term];
}

double DistributionImpl::logTermProb(std::string term, double collectionProb) {
  double value = DistributionImpl::_termProb[term];
  if(value > 0)
    return log(value);
  else 
    return 0;
}

double DistributionImpl::termFreq(std::string term) {
  return DistributionImpl::_termFreq[term];
}

double DistributionImpl::collectionProb(std::string term) {
  return DistributionImpl::_collProb[term];
}
