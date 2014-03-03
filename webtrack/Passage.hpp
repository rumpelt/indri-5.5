#ifndef PASSAGE_HPP
#define PASSAGE_HPP

#include <string>
#include <vector>
#include <map>

class Passage  {
private:
  unsigned long docId;
  std::string psgId; // The is sequential and can be used to identify the next passage.
  std::string trecId;
  int passageSize;
  int windowSize; // This is the overlapping window size
  float score;
  std::string content;
  std::vector<std::string> terms;
  std::map<std::string, int> termFreq;
  std::map<std::string, float> tfIdf; // can also be used for other purposes
public:
  void setWindowSize(int psgSize, int windowSize);
  void setTerms(std::vector<std::string> terms);
  void setPsgId(int psgId);
  void setDocId(unsigned long docId);
  void setScore(float score);
  void setTrecId(std::string trecId);
  void pushTerm(std::string term);
  void addTerms(std::vector<std::string>& terms);
  void crtTermFreq();
  int getPsgSz();
  float getScore();
  unsigned long getDocId();
  std::string getPsgId();
  std::vector<std::string> getTerms();
  std::string getTrecId();
  std::map<std::string, int> getTermFreq();

  std::map<std::string, float> getTfIdf(); // a map of terms to float values. to be used for various purpuses.
  int freq(std::string term);
  void printPassage();
  Passage(int psgSize, int windowSize);
  Passage();
};

#endif
