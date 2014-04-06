#include "PoissonDistribution.hpp"
#include "MathRoutines.hpp"
#include <stdexcept>
#include <climits>

PoissonDistribution::PoissonDistribution(double mu) : _mu(mu) {}

PoissonDistribution::PoissonDistribution() : _mu(2500) {}
PoissonDistribution::~PoissonDistribution(){}
/**
 * In case of factorial of large values we just use the maximum possible of unsigned long;
 */
double PoissonDistribution::termProb(std::string term, double collectionProb) {
  double lambdaEnh = 0;

  if(PoissonDistribution::_lambdaEstimate.find(term) != PoissonDistribution::_lambdaEstimate.end())
    lambdaEnh = PoissonDistribution::_lambdaEstimate[term] * PoissonDistribution::_size;
  else {
    lambdaEnh = (PoissonDistribution::_mu * collectionProb) / (PoissonDistribution::_size + PoissonDistribution::_mu);
    lambdaEnh = lambdaEnh * PoissonDistribution::_size;   
  }
      
  double lambdaPow = pow(lambdaEnh, PoissonDistribution::termFreq(term));
  double lambdaExp = exp(lambdaEnh);
  unsigned long fact = math::factorial(PoissonDistribution::_freqMap[term]);
  double termProb = 0;
  if(fact == 0 && PoissonDistribution::_freqMap[term] > 1)
    termProb = lambdaPow / ULONG_MAX; 
  else 
    termProb = lambdaPow / (lambdaExp * fact); 
  return termProb;
}

double PoissonDistribution::termFreq(std::string term) {
  if(PoissonDistribution::_freqMap.find(term) != PoissonDistribution::_freqMap.end() )
    return PoissonDistribution::_freqMap[term];
  else
    return 0;
}

double PoissonDistribution::collectionProb(std::string term) {
  if(PoissonDistribution::_collectionProb.find(term) != PoissonDistribution::_collectionProb.end() )
    return PoissonDistribution::_collectionProb[term];
  else
    return 0;
}

double PoissonDistribution::logTermProb(std::string term , double collectionProb) {
  double lambdaEnh = 0;
  if(PoissonDistribution::_lambdaEstimate.find(term) != PoissonDistribution::_lambdaEstimate.end())
    lambdaEnh = PoissonDistribution::_lambdaEstimate[term] * PoissonDistribution::_size;
  else {
    double estimate = (PoissonDistribution::_mu * collectionProb) / (PoissonDistribution::_size + PoissonDistribution::_mu);
    lambdaEnh = estimate * PoissonDistribution::_size;
  }
  if (lambdaEnh > 0)  {
    double lambdaPow =  PoissonDistribution::termFreq(term) * log(lambdaEnh);
    double logTermProb = lambdaPow - (( lambdaEnh * 1 ) + math::logFactorial(PoissonDistribution::termFreq(term)));
    return logTermProb;
  }
  else 
    return 0;
}

void PoissonDistribution::initialize(std::map<std::string, unsigned long>& freqMap, std::map<std::string, double>& collectionProb, unsigned long docSize) {
  PoissonDistribution::_size = docSize;
  PoissonDistribution::_freqMap = freqMap;
  PoissonDistribution::_collectionProb = collectionProb;
  for(std::map<std::string, unsigned long>::const_iterator termIt = freqMap.begin(); termIt != freqMap.end(); ++termIt) {
    std::string term = termIt->first;
    double freq = termIt->second;
    double collProb = 0;
    try {
      collProb = collectionProb.at(term); 
    }
    catch(std::out_of_range& expt) {
    }
    double lambdaEstimate = (freq + (PoissonDistribution::_mu * collProb)) / (PoissonDistribution::_size + PoissonDistribution::_mu);
    PoissonDistribution::_lambdaEstimate[term] = lambdaEstimate;
  }
}
