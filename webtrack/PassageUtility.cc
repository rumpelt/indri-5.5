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
std::map<std::string, int> passageutil::tfIdf(Passage* psg, indri::api::QueryEnvironment* qe, unsigned long totalDocs, std::map<std::string, double>& docCountMap, bool stem) {
  std::map<std::string, int> tfIdf;
  std::map<std::string, int> termFreq = psg->getTermFreq();
   for(std::map<std::string, int>::iterator mapIt = termFreq.begin(); mapIt != termFreq.end(); ++mapIt) {
    std::string term = mapIt->first;
    int freq = mapIt->second;
    double docCount = 0.00000;
    if(docCountMap.find(term) == docCountMap.end()) {
      if(stem)
        docCount = qe->documentStemCount(term);
      else
        docCount = qe->documentCount(term);
      docCountMap[term] = docCount;
    }
    else {
      docCount = docCountMap[term];
    }
    double idf = 0.00000;
    if (docCount > 0.00000) {
    //    std::cout << "term " << term << " " << docCount << " " << freq << "\n";
      idf = log(totalDocs/docCount) * freq;
    }
    //    std::cout << "term " << term << " Doc count " << docCount << " freq  " << freq  << " Total docs " << totalDocs << " idf " << idf << "\n";
    tfIdf.insert(std::pair<std::string, int>(term, (int)(idf+0.5))); // rounding done 
  }
  return tfIdf;
}

void  passageutil::psgSimilarityMatrix(std::vector<Passage*>& psgs, Passage* motherPassage, indri::api::QueryEnvironment& qe, std::map<std::string, double>& docCountMap) {
  TextMatrix* psgTm = 0;
  psgTm = passageutil::makeWordPassageMatrix(psgs);
  unsigned long  totalDocs = qe.documentCount();

  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    std::string id = psg->getPsgId();
    std::map<std::string, int> tfIdf = passageutil::tfIdf(psg, &qe, totalDocs, docCountMap, false);
    for(std::map<std::string, int>::iterator mapIt = tfIdf.begin(); mapIt != tfIdf.end(); ++mapIt) {
      std::string term = mapIt->first;
      
      float value = mapIt->second;
      psgTm->setValue(term, id, value);      
    } 
  }  
  Eigen::MatrixXf* matrix = psgTm->getMatrix();
  Eigen::MatrixXf transpose = matrix->transpose();
  Eigen::MatrixXf multi = transpose * (*matrix);
  
  std::cout << "Passage Matrix " << " " << multi.rows() << " " << multi.cols() << std::endl;
  std::cout << multi << std::endl;
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigenSolver(multi);
  std::cout << "Eigen Value " << eigenSolver.eigenvalues() << std::endl;
  std::cout << "Eigne vector " << eigenSolver.eigenvectors() << std::endl;
  if(psgTm != 0)
    delete psgTm;
}


float  passageutil::passageSimilarity(Passage& psgone, Passage& psgtwo, std::set<std::string>& vocab) {
  if(vocab.size() <= 0)  {
    std::vector<std::string> terms = psgone.getTerms();
    std::vector<std::string> secondTerms = psgone.getTerms();
    terms.insert(terms.end(), secondTerms.begin(), secondTerms.end());
    for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt)
      vocab.insert(*termIt);
  }
  TextMatrix tmOne;
  std::string idOne = "one";
  tmOne.initializeMatrix(vocab, idOne, 0);
  std::vector<std::string> terms = psgone.getTerms();
  for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
    std::string term = *termIt;
    tmOne.setValue(term, idOne, 1);
  }

  Eigen::VectorXf  oneV = (Eigen::VectorXf)((*(tmOne.getMatrix())).col(0));
  TextMatrix tmTwo;
  std::string idTwo = "two";
  tmTwo.initializeMatrix(vocab, idTwo, 0);
  terms = psgtwo.getTerms();
  for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
    std::string term = *termIt;
    tmTwo.setValue(term, idTwo, 1);
  }

  Eigen::VectorXf  twoV = (Eigen::VectorXf)((*(tmTwo.getMatrix())).col(0));
  Eigen::VectorXf prod = oneV.transpose() * twoV;
  float dot = prod(0) / (twoV.norm() * oneV.norm());
  return dot;   
}


float passageutil::psgCosineSimilarity(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe , unsigned long totalDocs, std::map<std::string, double>& docCountMap) {
  if (psgs.size() <= 0)
    return 0;
  TextMatrix* psgTm = 0;
  psgTm = passageutil::makeWordPassageMatrix(psgs);
  
  Eigen::MatrixXf mom(psgTm->getNumRows(), 1);
  mom.fill(0);
  
  std::map<std::string, int> motherTfIdf = passageutil::tfIdf(motherPassage, qe, totalDocs, docCountMap, false);
  for(std::map<std::string, int>::iterator mapIt = motherTfIdf.begin(); mapIt != motherTfIdf.end(); ++mapIt) {
    std::string term = mapIt->first;
    int value = mapIt->second; // rounding
    int rIdx = psgTm->rIndex(term);
    mom(rIdx,0) = value;
  }

  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    std::string id = psg->getPsgId();
    std::map<std::string, int> tfIdf = passageutil::tfIdf(psg, qe, totalDocs, docCountMap, false);
    for(std::map<std::string, int>::iterator mapIt = tfIdf.begin(); mapIt != tfIdf.end(); ++mapIt) {
      std::string term = mapIt->first;
      float value = mapIt->second;
      psgTm->setValue(term, id, value);      
    } 
  }  

  Eigen::MatrixXf psgMat = *(psgTm->getMatrix());
  //std::cout << "Passage Matrix ";
  //std::cout << psgMat.cols() << " " << psgMat.rows()  << "\n";
  //std::cout << "Mom Matrixt" << "\n";
  //std::cout << mom << "\n";
  Eigen::MatrixXf dotMat = psgMat.transpose() * mom;  
  //  assert(dotMat.cols() == 1);
  Eigen::VectorXf momV = (Eigen::VectorXf)(mom.col(0));
  float momSize = momV.norm();
  float sim = 0;
  for(size_t idx =0; idx < psgs.size(); ++idx) {
    Eigen::VectorXf psgCol = (Eigen::VectorXf)(psgMat.col(idx));
    float psgLen = psgCol.norm();
    float cosine = dotMat(idx,0) / (psgLen * momSize);
    sim = sim + cosine;
  }

  delete psgTm;
  return (sim/psgs.size());
}

float passageutil::psgSimpleCosineSimilarity(std::vector<Passage*>& psgs, Passage* motherPassage) {
  if (psgs.size() <= 0)
    return 0;
  TextMatrix* psgTm = 0;
  psgTm = passageutil::makeWordPassageMatrix(psgs);
  float sim;
  Eigen::MatrixXf mom(psgTm->getNumRows(), 1);
  mom.fill(0);
  
  std::vector<std::string> terms = motherPassage->getTerms();
  for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
    std::string term = *termIt;
    int rIdx = psgTm->rIndex(term);
    mom(rIdx,0) = 1;
  }

  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    std::string id = psg->getPsgId();
    std::vector<std::string> terms = psg->getTerms();
    for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
      std::string term = *termIt;
      psgTm->setValue(term, id, 1);      
    } 
  }  

  Eigen::MatrixXf psgMat = *(psgTm->getMatrix());
  //std::cout << "Passage Matrix ";
  //std::cout << psgMat.cols() << " " << psgMat.rows()  << "\n";
  //std::cout << "Mom Matrixt" << "\n";
  //std::cout << mom << "\n";
  Eigen::MatrixXf dotMat = psgMat.transpose() * mom;  
  //  assert(dotMat.cols() == 1);
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
float passageutil::docPsgHomogeniety(std::vector<Passage*> psgs, Passage* motherPassage, indri::api::QueryEnvironment* qe, unsigned long totalDocs, std::map<std::string,double>& docCountMap, bool stem) {

  int numPassages = psgs.size();
  std::map<std::string, int> motherTfIdf = passageutil::tfIdf(motherPassage, qe, totalDocs, docCountMap, stem);
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
    std::map<std::string, int> tfidf = passageutil::tfIdf(psg, qe, totalDocs, docCountMap, stem);

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
TextMatrix* passageutil::makePassageMatrix(Passage& psg) {
  std::set<std::string> 

}
*/

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
    std::vector<std::string> terms = psg->getTerms();

    for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
      vocabulary.insert(*termIt);   
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
