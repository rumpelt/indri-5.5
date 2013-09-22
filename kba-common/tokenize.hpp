#include <stdio.h>
#include <set>
#include <string>
#include <vector>
#include <iostream>
#include "boost/tokenizer.hpp"

class Tokenize {
  
public:
  static std::set<std::string> getStopSet(std::string& stopFile);
  static std::vector<std::string> filterStopWords(std::vector<std::string> inputTokens, std::set<std::string>& stopwords);

  static std::vector<std::string> getPhrases(std::string& inputSource);

};
