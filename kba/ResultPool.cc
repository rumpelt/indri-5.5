#include "ResultPool.hpp"
#include <iostream>

ResultPool::ResultPool(std::string id, int poolSz, int initScore, bool biggerIsBetter): id(id), poolSz(poolSz), biggerIsBetter(biggerIsBetter) {
  for (int idx=0; idx < poolSz; ++idx) {
    ResultStruct* rs = new ResultStruct(initScore);
    results.push_back(rs);
  }
  cndPtr = 0;
}




ResultPool::~ResultPool() {
  for(std::vector<ResultStruct*>::iterator resIt = results.begin(); resIt != results.end(); ++resIt) {
    delete *resIt;
  }
  results.clear();
}
