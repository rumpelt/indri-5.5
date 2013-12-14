#include "PassageUtility.hpp"
#include <math.h>
#include <assert.h>

/**
 *  length based homogeniety
 */
float passageutil::lenHomogeniety(int minLength, int maxLength, int docLength) {  
  float minLog = log(minLength);
  float maxLog  = log(maxLength);
  return (1 - ((log(docLength) - minLog) / (maxLog - minLog)));
}

/**
 * assume that termFreq map of Passage is populated.
 * Not tested yet
 */
std::map<std::string, int> passageutil::tfIdf(Passage* psg, indri::api::QueryEnvironment* qe, bool stem) {
  std::map<std::string, int> tfIdf;
  unsigned long totalDocs = qe->documentCount();
  assert(totalDocs > 0);
  std::map<std::string, int> termFreq = psg->getTermFreq();
  for(std::map<std::string, int>::iterator mapIt = termFreq.begin(); mapIt != termFreq.end(); ++mapIt) {
    std::string term = mapIt->first;
    int freq = mapIt->second;
    unsigned long docCount;
    if(stem)
      docCount = qe->documentStemCount(term);
    else
      docCount = qe->documentCount(term);

    double idf = log(totalDocs/docCount) * freq;
    tfIdf.insert(std::pair<std::string, int>(term, (int)idf)); 
  }
  return tfIdf;
}

/**
 * No tested yet
 */
float passageutil::docPsgHomogeniety(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, bool stem) {

  int numPassages = psgs.size();
  std::map<std::string, int> motherTfIdf = passageutil::tfIdf(motherPassage, qe, stem);
  int motherLength = 0;
  for(std::map<std::string, int>::iterator termMapIt = motherTfIdf.begin(); termMapIt != motherTfIdf.end(); ++termMapIt) {
    int value = termMapIt->second;
    motherLength += (value * value);
  }
  motherLength = sqrt(motherLength);
  float totalScore = 0;  
  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt !=  psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    float cosineScore = 0;
    std::map<std::string, int> tfidf = passageutil::tfIdf(psg, qe, stem);

    int length = 0;
    for(std::map<std::string, int>::iterator termMapIt = tfidf.begin(); termMapIt != tfidf.end(); ++termMapIt) {
      int value = termMapIt->second;
      std::string term = termMapIt->first;
      length += (value * value);
      try {
        int motherValue = motherTfIdf[term];
        cosineScore += (value * motherValue);
      }
      catch(std::out_of_range& expt) {
      }
    }
    length = sqrt(length); 
    totalScore =  totalScore + (cosineScore / (length * motherLength));
  }
  return (totalScore / numPassages);
}
