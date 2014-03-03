#ifndef RESULTPOOL_HPP
#define RESULTPOOL_HPP

#include <string>
#include <vector>

struct ResultStruct {
  struct greater {
    /**
    bool operator() (ResultStruct* one, ResultStruct* two ) {
      return one->score < two->score;
    }
    */
    bool operator() (ResultStruct& one, ResultStruct& two ) {
      return one.score < two.score;
    }
  };
  struct lesser {
    bool operator() (ResultStruct& one, ResultStruct& two ) {
      return one.score > two.score;
    }
  };
  std::string id; // this is document identifier;
  std::string dayDt;
  int score;
  float origScore; // Score obtained by some other tool eg. Indri
  ResultStruct(float initSc) : score(initSc) {}
};

class ResultPool {
private:
  std::string id;
  int poolSz;
  int cndPtr; // Candidate Pointer for replacement
  bool biggerIsBetter;
  std::vector<ResultStruct*> results;
public:
  /**
   * true value to gtcmp mean bigger integer value represent bigger score and
   * so we would like to replace the min value in result pool;
   * Whereas false value represent smallere integer value represent better score and
   * so we would like to replace the max value in the pool;
   */
  bool addResult(std::string id, std::string dayDt, int sc);    
  void correctCndPtr();
  std::vector<ResultStruct*> getResults();
  ResultPool(std::string, int poolSz, int initSc, bool biggerIsBetter=true);
  ~ResultPool();
};
inline std::vector<ResultStruct*> ResultPool::getResults() {return results;};

inline bool ResultPool::addResult(std::string id, std::string dayDt, int sc) {
  ResultStruct* minRs = results[ResultPool::cndPtr];
  if(ResultPool::biggerIsBetter && minRs->score < sc) { 
    minRs->id = id;
    minRs->dayDt = dayDt;
    minRs->score = sc;
    
    ResultPool::correctCndPtr();
    return true;
  }
  else if ((!ResultPool::biggerIsBetter) && minRs->score > sc) {
    minRs->id = id;
    minRs->dayDt = dayDt;
    minRs->score = sc;
    ResultPool::correctCndPtr();
    return true;
  }
  else
    return false;
}
inline void ResultPool::correctCndPtr() {
  float score = results[0]->score;
  int idx = 0;
  cndPtr = idx;
  for(; idx < results.size(); ++idx) {
    if(biggerIsBetter && results[idx]->score < score) {
      ResultPool::cndPtr = idx;
      score = results[idx]->score;
    }
    else if ((!ResultPool::biggerIsBetter) && results[idx]->score > score) {
      ResultPool::cndPtr = idx;
      score = results[idx]->score;
    }
  }
}

#endif 
