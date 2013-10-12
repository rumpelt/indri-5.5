#include "ParsedStream.hpp"

#include <stdexcept>

void updateTermBase(kba::stream::ParsedStream* parsedStream, kba::term::TermBase* termBase, std::set<std::string> termSet) {
  size_t docSize= (parsedStream->tokens).size();
  if( docSize > 0) {
    termBase->totalDocs = termBase->totalDocs + 1;
    termBase->totalDocLength = termBase->totalDocLength +  docSize;
    termBase->avgDocLength = (1.0 * termBase->totalDocLength) / termBase->totalDocs;
  }
  for(std::set<std::string>::iterator termIt = termSet.begin(); termIt != termSet.end(); ++termIt) {
    std::string term = *termIt;
    unsigned int value = 1;
    if((parsedStream->tokenSet).count(term) > 0) {
      try {
	value  = (termBase->termDocFreq).at(term);
        (termBase->termDocFreq).erase(term);
        (termBase->termDocFreq).insert(std::pair<std::string, unsigned int>(term, value+1));
         
      } catch(std::out_of_range& oor) {
        (termBase->termDocFreq).insert(std::pair<std::string, unsigned int>(term, value));
      }

      float idf = log((termBase->totalDocs - value + 0.5 )/ (value + 0.5)) ;
      idf = idf/kba::term::LOG2;
      if(idf < 0.0) // In case IDF is less than zero..This can happe
        idf = 0.0;
      (termBase->logIDF).erase(term);
      (termBase->logIDF).insert(std::pair<std::string, float> (term, idf));
      
    } 
  }
}
  
void kba::stream::populateTokenFreq(kba::stream::ParsedStream* parsedStream) {
  if((parsedStream->tokens).size() > 0) {
    
    for (std::vector<std::string>::iterator tokIt = (parsedStream->tokens).begin(); tokIt != (parsedStream->tokens).end(); ++tokIt) {
      std::string term = *tokIt;
      try {
	
        int value = (parsedStream->tokenFreq).at(term);
        value = value + 1;
        (parsedStream->tokenFreq).erase(term);
        (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, value));
      }
      catch(std::out_of_range& oor) {
        (parsedStream->tokenFreq).insert(std::pair<std::string, int>(term, 1));
      }
    }
  }
}
