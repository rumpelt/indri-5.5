#include "PassageUtility.hpp"
#include <math.h>
#include <assert.h>
#include <Eigen/Eigenvalues>

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
    //    std::cout << "term " << term << " " << docCount << " " << freq << "\n";
    double idf = log(totalDocs/docCount) * freq;
    
    tfIdf.insert(std::pair<std::string, int>(term, (int)(idf+0.5))); // rounding done 
  }
  return tfIdf;
}

float passageutil::psgCosineSimilarity(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe) {
  if (psgs.size() <= 0)
    return 0;
  TextMatrix* psgTm = 0;
  psgTm = passageutil::makeWordPassageMatrix(psgs);
  float sim;
  Eigen::MatrixXf mom(psgTm->getNumRows(), 1);
  mom.fill(0);
  std::map<std::string, int> motherTfIdf = passageutil::tfIdf(motherPassage, qe);
  for(std::map<std::string, int>::iterator mapIt = motherTfIdf.begin(); mapIt != motherTfIdf.end(); ++mapIt) {
    std::string term = mapIt->first;
    int value = mapIt->second; // rounding
    int rIdx = psgTm->rIndex(term);
    mom(rIdx,0) = value;
  }

  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    std::map<std::string, int> tfIdf = passageutil::tfIdf(psg, qe);
    for(std::map<std::string, int>::iterator mapIt = tfIdf.begin(); mapIt != tfIdf.end(); ++mapIt) {
      std::string term = mapIt->first;
      psgTm->setValue(term, psg->getPsgId(), mapIt->second);      
    } 
  }  

  Eigen::MatrixXf psgMat = *(psgTm->getMatrix());
  //std::cout << "Passage Matrix ";
  //std::cout << psgMat.cols() << " " << psgMat.rows()  << "\n";
  //std::cout << "Mom Matrixt" << "\n";
  //std::cout << mom << "\n";
  Eigen::MatrixXf dotMat = psgMat.transpose() * mom;  
  assert(dotMat.cols() == 1);
  Eigen::VectorXf momV = (Eigen::VectorXf)(mom.col(0));
  float momSize = momV.norm();
  
  for(size_t idx =0; idx < psgs.size(); ++idx) {
    Eigen::VectorXf psgCol = (Eigen::VectorXf)(psgMat.col(idx));
    float psgLen = psgCol.norm();
    float cosine = dotMat(idx,0) / (psgLen * momSize);
    sim = sim + cosine;
  }

  delete psgTm;
  return (sim/psgs.size());
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

/**
 * caller must delete the text matrix returned here
 * Assume that term freq map of Passage is populated.
 */
TextMatrix*  passageutil::makeWordPassageMatrix(std::vector<Passage*> psgs) {
  std::set<std::string> vocabulary;
  std::set<std::string> psgIds;
  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    std::string id = psg->getPsgId();
    psgIds.insert(id);
    std::map<std::string, int> tfidf = psg->getTermFreq();
    assert(tfidf.size() > 0);
    for(std::map<std::string,int>::iterator mapIt = tfidf.begin(); mapIt != tfidf.end(); ++mapIt) {
      std::string term = mapIt->first;
      vocabulary.insert(term);   
    }
  }
  TextMatrix* tm = new TextMatrix();
  tm->initializeMatrix(vocabulary, psgIds,0);    
  return tm;     
}

void passageutil::computeEigenVector(TextMatrix* tm) {
  Eigen::MatrixXf* tmat = tm->getMatrix();
  Eigen::MatrixXf m = (*tmat) * tmat->transpose();
  Eigen::EigenSolver<Eigen::MatrixXf> eg(m);
  std::cout << eg.eigenvectors();      
}
