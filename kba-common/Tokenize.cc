#include "Tokenize.hpp"
#include <fstream>

std::unordered_set<std::string> Tokenize::getStopSet(std::string& stopFile) {
  std::unordered_set<std::string> stopwords;
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

std::vector<std::string> Tokenize::toLower(std::vector<std::string>& inVector) {
  std::vector<std::string> lowerVector;
  for(std::vector<std::string>::iterator vecIt = inVector.begin(); vecIt != inVector.end(); vecIt++) {
    std::string lowerString = *vecIt;
    
    std::transform(lowerString.begin(), lowerString.end(), lowerString.begin(), ::tolower); 
    lowerVector.push_back(lowerString);
  }
  return lowerVector;
}

std::vector<std::string> Tokenize::ngrams(std::vector<std::string>& inVector, int ngram) {
  std::vector<std::string> gramVector;
  if(ngram <= 1) 
    return inVector;
  int upLimit = inVector.size() - ngram + 1;

  for(int index = 0; index < upLimit ; index++) {
    std::string grams = inVector[index];
    for(int count = 1; count < ngram;count++) {
      grams = grams+ " "+ inVector[index + count];
    }
    gramVector.push_back(grams);
  }
  return gramVector;
}

std::vector<std::string> Tokenize::filterStopWords(std::vector<std::string>& inputTokens, std::unordered_set<std::string>& stopwords) 
{
  if(stopwords.size() <= 0)
    return inputTokens;
  std::vector<std::string> filterWords;
 
  for(std::vector<std::string>::iterator tokIt = inputTokens.begin(); tokIt != inputTokens.end(); tokIt++) {
    std::string token = *tokIt;
    if(token.size() > 1 && stopwords.find(token) == stopwords.end()) {
      filterWords.push_back(token);
    }
  }
  return filterWords;
}

std::vector<std::string> Tokenize::filterShortWords(std::vector<std::string>& inputTokens, int lengthToReject) 
{
  std::vector<std::string> filterWords;
  for(std::vector<std::string>::iterator tokIt = inputTokens.begin(); tokIt != inputTokens.end(); tokIt++) {
    std::string token = *tokIt;
    if(token.size() > lengthToReject) {
      filterWords.push_back(token);
    }
  }
  return filterWords;
}


std::vector<std::string> Tokenize::tokenize(std::string& inputSource) {
  std::vector<std::string> tokens;
  char phrase[4096];
  int phraseIndex=0;
  bool prevUpperCase = false;
  for(std::string::iterator charIt = inputSource.begin(); charIt != inputSource.end(); charIt++) {
    char thisChar = *charIt;
    if(!isspace(thisChar)) { // If this case is not upper then we process it
      if(!prevUpperCase && isupper(thisChar)) { // IF the previous character was also upper case then it might be some abbreviation and so we just append it to previous

	std::string content((const char*) &phrase, phraseIndex);
        if(content.size() > 0)
          tokens.push_back(content);
        phraseIndex = 0;
        phrase[phraseIndex] = thisChar;
        phraseIndex++;  

      }
      else if(Tokenize::isSpecialChar(thisChar)) {
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
      //      std::cout << "Tokenize.cc : function tokenize, we have used insufficenet array storage, fix this with dynamic memory allocation= inputString :" << inputSource << "\n";
      //std::string content((const char*) &phrase, 4096);
      //if(content.size() > 0)
      //  tokens.push_back(content);
      phraseIndex = 0;
    }
    prevUpperCase = isupper(thisChar);
  }
  
  if(phraseIndex > 0) {
    std::string content((const char*) &phrase, phraseIndex);
    tokens.push_back(content);
  }
 
  return tokens;
}

/**
 * use the boost tokenizer.
 */
std::vector<std::string> Tokenize::getPhrases(std::string& inputSource) {
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



