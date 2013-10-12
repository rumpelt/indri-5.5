#ifndef TOKENIZE_HPP
#define TOKENIZE_HPP

#include <stdio.h>
#include <unordered_set>
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include "boost/tokenizer.hpp"

class Tokenize {
  
public:
  static std::unordered_set<std::string> getStopSet(std::string& stopFile);
  static std::vector<std::string> filterStopWords(std::vector<std::string>& inputTokens, std::unordered_set<std::string>& stopwords);
  static std::vector<std::string> filterShortWords(std::vector<std::string>& inputTokens, int lengthToReject=1);

  static std::vector<std::string> getPhrases(std::string& inputSource);

  /**
   * Tokenize a  string on whitespace, non alpha numeric characters and upper case letters;
   */
  static std::vector<std::string> tokenize(std::string& inputSource);
  static std::vector<std::string> ngrams(std::vector<std::string>& inVector, int ngram);
  static std::vector<std::string> toLower(std::vector<std::string>& inVector);
};
#endif
