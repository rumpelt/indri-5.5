#ifndef PASSAGEUTILITY_HPP
#define PASSAGEUTILITY_HPP
#include "Passage.hpp"
#include "indri/QueryEnvironment.hpp"
#include <map>
#include <unordered_set>
#include "TextMatrix.hpp"

namespace passageutil{
  float lenHomogeniety(int minLength, int maxLength, int docLength);
  std::map<std::string, int> tfIdf(Passage* psg, indri::api::QueryEnvironment* qe, unsigned long totalDocs, std::map<std::string, double>& docCountMap, bool stem);

  float docPsgHomogeniety(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, unsigned long totalDocs, std::map<std::string, double>& docCountMap, bool stem);
  float  passageSimilarity(Passage& psgone, Passage& psgtwo, std::set<std::string>& vocab);
  void psgSimilarityMatrix(std::vector<Passage*>& psgs, Passage* motherPassage, indri::api::QueryEnvironment& qe, std::map<std::string, double>& docCountMap);
  float psgCosineSimilarity(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, unsigned long totalDocs, std::map<std::string, double>& docCountMap);
  float psgSimpleCosineSimilarity(std::vector<Passage*>& psgs, Passage* motherPassage);  
  TextMatrix* makeWordPassageMatrix(std::vector<Passage*> psgs);
  /**
   * First transpose matrix and then find the eigen vector
   */
  void computeEigenVector(TextMatrix* tm);
};
#endif
