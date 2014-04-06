#include "MathRoutines.hpp"
#include "PoissonModel.hpp"


#include <cmath>
PoissonModel::PoissonModel(float mu) : _mu(mu) {}

PoissonModel::PoissonModel() : _mu(2500) {}


void PoissonModel::setCollectionSize(unsigned long collSize) {
  //  std::cout << "Setting coll size " << collSize << std::endl;
  PoissonModel::_collectionSize = collSize;
}

void PoissonModel::setCollectionFreq(std::string term, unsigned long freq) {
  PoissonModel::_collectionFreq[term] =  freq;
}

float PoissonModel::score(std::vector<std::string>& query, Passage* psg) {
   
  float psgSize = psg->getPsgSz();
  float score = 0;
  //  std::cout << "Scoring " << psgSize << std::endl;
  for (std::vector<std::string>::iterator qIt = query.begin(); qIt != query.end(); ++qIt) {
    std::string term = *qIt;
    int freq = psg->freq(term);
    double logFactorial = math::logFactorial(freq);

    unsigned long g_freq = PoissonModel::_collectionFreq[term];
    if (freq == 0 && g_freq == 0) 
      g_freq = 1; // if global freq is zero then it is stop word or probably we were not able to fetch in the few indexes we were looking.

    float globalProb = (PoissonModel::_mu * g_freq) / PoissonModel::_collectionSize;
    double ip_freq = freq + globalProb;
    double lambda = (ip_freq/ (psgSize + PoissonModel::_mu)); 

    double lambdaEnh = lambda * psgSize;
    //double exponent = exp(lambdaEnh);
    //    double lambdaPow = pow(lambdaEnh, freq);
    if(freq == 0)
      freq = 1;
    double termScore = (freq * log(lambdaEnh)) - ((lambdaEnh * 1) + logFactorial);
    
    //std::cout << "Term " << term << " Psg fre " << freq << " global prb " << globalProb << " lambda " << lambda << " lambda enh " << lambdaEnh << " psg size " << psgSize << "lambda enh " << lambdaEnh << " "  << " termscore " << termScore << " factorial " << logFactorial <<std::endl;
    score = score + termScore;
  }

  //std::cout << "Score " << score << std::endl;
  return score;
}


/**
 * We are just not taking into account the other term present in the query.
 * not yet implemented.
 */
float PoissonModel::score(std::vector<std::string>& query, Passage* psg, std::map<std::string, unsigned long>& collFreq, const unsigned long& collSize) {
  return 0.0;
}
