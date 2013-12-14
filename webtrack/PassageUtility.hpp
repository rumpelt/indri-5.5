#ifndef PASSAGEUTILITY_HPP
#define PASSAGEUTILITY_HPP
#include "Passage.hpp"
#include "indri/QueryEnvironment.hpp"
#include <map>
#include <unordered_set>

namespace passageutil{
  float lenHomogeniety(int minLength, int maxLength, int docLength);
  std::map<std::string, int> tfIdf(Passage* psg, indri::api::QueryEnvironment* qe, bool stem);
  float docPsgHomogeniety(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, bool stem);
};
#endif
