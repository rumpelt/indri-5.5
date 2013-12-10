#ifndef LANGUAGEMODEL_HPP
#define LANGUAGEMODEL_HPP

#include "Passage.hpp"
#include <map>
class LanguageModel {
private:
  float _mu;
public:
  LanguageModel();
  float score(std::vector<std::string> query, Passage* psg, std::map<std::string, unsigned long> collFreq, unsigned long collSize);
};
#endif
