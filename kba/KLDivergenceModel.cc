#include "KLDivergenceModel.hpp"

float KLDivergenceModel::score(query_t& query, Distribution& psg) {
  float score = 0;
  std::vector<std::string>& textV = query.textVector;
  Distribution* qDist = query.distribution;
  for(std::vector<std::string>::const_iterator textIt = textV.begin(); textIt != textV.end(); ++textIt) {
    std::string term = *textIt;
    
    double prob = qDist->termProb(term , qDist->collectionProb(term)) * psg.logTermProb(term, qDist->collectionProb(term));
    //       std::cout << "term " << term << " Query Term Prob " << qDist->termProb(term, qDist->collectionProb(term)) << " document  prob " << psg.logTermProb(term, qDist->collectionProb(term)) << " coll pron " << qDist->collectionProb(term) << " Doc term frewq " <<  psg.termFreq(term) << std::endl;
    score = score + prob;
  }

  // score = -1 * score;
  //std::cout << "Score " << score << std::endl;
  return score;
}
