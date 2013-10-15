#include "ParsedStream.hpp"

#include <stdexcept>
#include <iostream>  
void kba::stream::populateTokenFreq(kba::stream::ParsedStream* parsedStream) {
  if((parsedStream->tokens).size() > 0) {
    
    for (std::vector<std::string>::iterator tokIt = (parsedStream->tokens).begin(); tokIt != (parsedStream->tokens).end(); ++tokIt) {
      std::string term = *tokIt;
      try {
        	
        int value = (parsedStream->tokenFreq).at(term);
        value = value + 1;
	std::cout << "Update term ";
        (parsedStream->tokenFreq).erase(term);
        (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, value));
      }
      catch(std::out_of_range& oor) {
        (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, 1));
      }
    }
  }
}
