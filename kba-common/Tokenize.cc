#include "Tokenize.hpp"
#include <fstream>
std::set<std::string> Tokenize::getStopSet(std::string& stopFile) {
  std::set<std::string> stopwords;
  std::ifstream stopWordFile(stopFile.c_str(),std::ifstream::in);
  if(stopWordFile.is_open()) {
    std::string line;
    while(std::getline(stopWordFile, line)){
      stopwords.insert(line);
    }
  }
  else {
    std::cout << "cannot open the stop word file";
  }
  return stopwords;
}

std::vector<std::string> Tokenize::filterStopWords(std::vector<std::string> inputTokens, std::set<std::string>& stopwords) 
{
  std::vector<std::string> filterWords;
  for(std::vector<std::string>::iterator tokIt = inputTokens.begin(); tokIt != inputTokens.end(); tokIt++) {
    std::string token = *tokIt;
    if(token.size() > 1 && stopwords.find(token) != stopwords.end()) {
      filterWords.push_back(token);
    }
  }
  return filterWords;
}

std::vector<std::string> Tokenize::tokenize(std::string& inputSource) {
  std::vector<std::string> tokens;
  char phrase[4096];
  int phraseIndex=0;
  for(std::string::iterator charIt = inputSource.begin(); charIt != inputSource.end(); charIt++) {
    char thisChar = *charIt;
    if(!isspace(thisChar)) {
      if(isupper(thisChar)) {
	std::string content((const char*) &phrase, phraseIndex);
        if(content.size() > 0)
          tokens.push_back(content);
        phraseIndex = 0;
        phrase[phraseIndex] = thisChar;
        phraseIndex++;  
      }
      else if(!isalnum(thisChar)) {
        std::string content((const char*) &phrase, phraseIndex);
        if(content.size() > 0)
          tokens.push_back(content);
        content = thisChar;
        tokens.push_back(content);
        phraseIndex = 0;
      }
      else {
        phrase[phraseIndex] = thisChar;
        phraseIndex++;  
      }
    }
    else  {
      std::string content((const char*) &phrase, phraseIndex);
      if(content.size() > 0)
        tokens.push_back(content);
      phraseIndex = 0;
    }
    if(phraseIndex >= 4096) {
      std::cout << "Tokenize.cc : function tokenize, we have used insufficenet array storage, fix this with dynamic memory allocation \n";
      std::string content((const char*) &phrase, 4096);
      if(content.size() > 0)
        tokens.push_back(content);
      phraseIndex = 0;
    }
  }
  
  if(phraseIndex > 0) {
    std::string content((const char*) &phrase, phraseIndex);
    tokens.push_back(content);
  }
 
  return tokens;
}

std::vector<std::string> getPhrases(std::string& inputSource) {
  boost::tokenizer<> tokens(inputSource);
  std::vector<std::string> phrases;
  std::vector<std::string>::iterator phraseIt;
  bool prevCaseUpper = false;
  for(boost::tokenizer<>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    bool upperCase = isupper(token[0]);
    if(!upperCase || !prevCaseUpper) {
      phrases.push_back(token); 
    }
    else {
      std::string lastElement = phrases.back();
      lastElement = lastElement + " "+token;
      phrases.pop_back();
      phrases.push_back(lastElement);
    }
    prevCaseUpper = upperCase;
  }

  return phrases;
}



