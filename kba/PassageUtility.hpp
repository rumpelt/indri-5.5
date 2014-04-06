#ifndef PASSAGEUTILITY_HPP
#define PASSAGEUTILITY_HPP
#include "Passage.hpp"
#include "indri/QueryEnvironment.hpp"
#include <map>
#include <unordered_set>
#include "TextMatrix.hpp"

namespace passageutil{
  float lenHomogeniety(int minLength, int maxLength, int docLength);
  std::map<std::string, int> tfIdf(Passage* psg, indri::api::QueryEnvironment* qe, bool stem = true);

  float docPsgHomogeniety(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, bool stem);

  float psgCosineSimilarity(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe);
  
  TextMatrix* makeWordPassageMatrix(std::vector<Passage*> psgs);
  /**
   * First transpose matrix and then find the eigen vector
   */
  void computeEigenVector(TextMatrix* tm);
};
#endif
