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


std::vector<std::string> Tokenize::split(std::string& inputSource) {
  std::vector<std::string> tokens;
  char phrase[4096];
  int phraseIndex=0;
  for(std::string::iterator charIt = inputSource.begin(); charIt != inputSource.end(); charIt++) {
    char thisChar = *charIt;
    if(!isspace(thisChar)) {
      phrase[phraseIndex] = thisChar;
      phraseIndex+=1;
    }
    else {
      std::string content((const char*) &phrase, phraseIndex);
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

std::vector<std::string> Tokenize::whiteSpaceSplit(std::string& inputSource, std::unordered_set<std::string> stopSet, bool lower, unsigned int charLimit, bool stem) {
  std::vector<std::string> tokens;
  char phrase[4096];
  int phraseIndex=0;
  stem::KrovetzStemmer * stemmer = new stem::KrovetzStemmer();   
  //  for(std::string::iterator charIt = inputSource.begin(); charIt != inputSource.end(); ++charIt) {
  for(size_t idx=0; idx < inputSource.size(); ++idx) {
    char thisChar = inputSource[idx];
    if(!isspace(thisChar)) {
      phrase[phraseIndex] = thisChar;
      phraseIndex+=1;
    }
    else {
      std::string content((const char*) &phrase, phraseIndex);
      if(content.size() > charLimit) {
        if(lower)
	  std::transform(content.begin(), content.end(), content.begin(), ::tolower);
        if(stopSet.find(content) == stopSet.end())  { 
          if (stem) { 
            char* term = (stemmer->kstem_stemmer((char*)(content.c_str())));
	    //	    std::cout << " term " << content << " stem " << term << "\n";
            tokens.push_back(term);
	  }
          else
            tokens.push_back(content);
	}
      }
      phraseIndex = 0;
    }
  }
  if(phraseIndex > 0) {
    std::string content((const char*) &phrase, phraseIndex);
    if(content.size() > charLimit) {
      if(lower)
        std::transform(content.begin(), content.end(), content.begin(), ::tolower);
      if(stopSet.find(content) == stopSet.end()) {
        if (stem) { 
            char* term = (stemmer->kstem_stemmer((char*)(content.c_str())));
            tokens.push_back(term);
	  }
          else
            tokens.push_back(content);
      }
    }
  }
  delete(stemmer);
  return tokens;
}

std::vector<std::string> Tokenize::tokenize(std::string& inputSource, bool lower, std::unordered_set<std::string>& stopwords, int minChar) {
  std::vector<std::string> tokens;
  char phrase[4096];
  int phraseIndex=0;
  bool prevUpperCase = false;
  for(std::string::iterator charIt = inputSource.begin(); charIt != inputSource.end(); charIt++) {
    char thisChar = *charIt;
    if(!isspace(thisChar)) { // If this case is not upper then we process it
      if(!prevUpperCase && isupper(thisChar)) { // IF the previous character was also upper case then it might be some abbreviation and so we just append it to previous
	std::string content((const char*) &phrase, phraseIndex);
        if(content.size() >= minChar) {
          if(lower)
            std::transform(content.begin(), content.end(), content.begin(), ::tolower);
          if(stopwords.find(content) == stopwords.end())
            tokens.push_back(content);
        }
        phraseIndex = 0;
        phrase[phraseIndex] = thisChar;
        phraseIndex++;  

      }
      else if(Tokenize::isSpecialChar(thisChar)) {
        std::string content((const char*) &phrase, phraseIndex);
        if(content.size() >= minChar) {
          if(lower)
            std::transform(content.begin(), content.end(), content.begin(), ::tolower);
          if(stopwords.find(content) == stopwords.end())
            tokens.push_back(content);
        }
        // Since we donot proces character of lenght less than 2 and so following two lines commented.
        /**
        content = thisChar;
        tokens.push_back(content);
	*/
        phraseIndex = 0;
      }
      else {
        phrase[phraseIndex] = thisChar;
        phraseIndex++;  
      }
    }
    else  {
      std::string content((const char*) &phrase, phraseIndex);
      if(content.size() >= minChar) {
        if(lower)
          std::transform(content.begin(), content.end(), content.begin(), ::tolower);
        if(stopwords.find(content) == stopwords.end())
          tokens.push_back(content);
      }
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
    if (content.size() >= minChar) {
      if(lower)
        std::transform(content.begin(), content.end(), content.begin(), ::tolower);
      if(stopwords.find(content) == stopwords.end())
        tokens.push_back(content);
    }
  }
 
  return tokens;
}

/**
 * use the boost tokenizer.
 */
std::vector<std::string> Tokenize::getPhrases(std::string& inputSource) {
  boost::tokenizer<> tokens(inputSource);
  std::vector<std::string> phrases;
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



