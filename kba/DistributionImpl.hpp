#ifndef DISTRIBUTIONIMPL_HPP
#define DISTRIBUTIONIMPL_HPP
#include "Distribution.hpp"
#include <map>

class DistributionImpl : public Distribution {
private:
  std::map<std::string, double> _termProb;
  std::map<std::string, double> _termFreq;
  std::map<std::string, double> _collProb;
public:
  void setTermProb(std::map<std::string, double> termProb);
  void setTermFreq(std::map<std::string, double> termFreq);
  void setCollectionProb(std::map<std::string,double> collProb);
  double termProb(std::string term, double collectionProb);
  double logTermProb(std::string term, double collectionProb);
  double termFreq(std::string term);
  double collectionProb(std::string term) ;
};

#endif
