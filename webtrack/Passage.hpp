#ifndef PASSAGE_HPP
#define PASSAGE_HPP

#include <string>
#include <vector>
#include <map>

class Passage  {
private:
  int docId;
  int psgId;
  std::string trecId;
  float score;
  std::string content;
  std::vector<std::string> terms;
  std::map<std::string, int> termFreq;
public:
  void setTerms(std::vector<std::string> terms);
  void setPsgId(int psgId);
  void setDocId(int docId);
  void setScore(float score);
  void setTrecId(std::string trecId);
  void crtTermFreq();
  int getPsgSz();
  float getScore();
  int getDocId();
  std::vector<std::string> getTerms();
  std::string getTrecId();
  int freq(std::string term);
};

#endif
