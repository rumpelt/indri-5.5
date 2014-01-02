
#ifndef PASSAGE_HPP
#define PASSAGE_HPP

#include <string>
#include <vector>
#include <map>

class Passage  {
private:
  unsigned long docId;
  std::string psgId;
  std::string trecId;
  float score;
  std::string content;
  std::vector<std::string> terms;
  std::map<std::string, int> termFreq;
  std::map<std::string, float> tfIdf; // can also be used for other purposes
public:
  void setTerms(std::vector<std::string> terms);
  void setPsgId(int psgId);
  void setDocId(unsigned long docId);
  void setScore(float score);
  void setTrecId(std::string trecId);
  void pushTerm(std::string term);
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
};

#endif
