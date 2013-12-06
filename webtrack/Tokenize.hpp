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
  static bool isSpecialChar(char ch);
  static std::vector<std::string> split(std::string& inputSource);
  static std::vector<std::string> tokenize(std::string& inputSource, bool lower, std::unordered_set<std::string>& stopwords);
  static std::vector<std::string> ngrams(std::vector<std::string>& inVector, int ngram);
  static std::vector<std::string> toLower(std::vector<std::string>& inVector);
};

inline  bool Tokenize::isSpecialChar(char ch) {
  return ((ch >= 0x21 && ch  <= 0x2f) || (ch >= 0x3a && ch <= 0x40) || (ch >= 0x5b && ch <= 0x60) || (ch >= 0x7b && ch <= 0x7e)) ? true : false;
}

inline std::vector<std::string> Tokenize::toLower(std::vector<std::string>& inVector) {
  std::vector<std::string> lowerVector;
  for(std::vector<std::string>::iterator vecIt = inVector.begin(); vecIt != inVector.end(); vecIt++) {
    std::string lowerString = *vecIt;
    
    std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower); 
    lowerVector.push_back(lowerString);
  }
  return lowerVector;
}

inline std::vector<std::string> Tokenize::filterShortWords(std::vector<std::string>& inputTokens, int lengthToReject) 
{
  std::vector<std::string> filterWords;
  for(std::vector<std::string>::iterator tokIt = inputTokens.begin(); tokIt != inputTokens.end(); tokIt++) {
    std::string token = *tokIt;
    if(token.size() > (size_t)lengthToReject) {
      filterWords.push_back(token);
    }
  }
  return filterWords;
}

#endif
