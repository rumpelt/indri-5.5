#ifndef STATDB_HPP
#define STATDB_HPP
 
#include "stdio.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "TermDict.hpp"
#include "boost/shared_ptr.hpp"
#include "Logging.hpp"


using namespace kba::term;
class StatDb {
private:
  int _trmFd;
  int _trmDtWritten;
  int _crpFd;
  int _crpDtWritten;
  int _tpcFd;
  int _trmTpcFd;
  int _evalFd;
  int _pgSz; // pagesize  
  int _strmFd;

public:
  StatDb();
  ~StatDb();

  void crtTrmDb(std::string fName, bool rdOnly);
  void crtCrpDb(std::string fName, bool rdOnly);
  void crtTpcDb(std::string fName, bool rdOnly);
  void crtTrmTpcDb(std::string fName, bool rdOnly);
  void crtEvalDb(std::string fName, bool rdOnly);
  void crtStreamDb(std::string fName, bool rdOnly); 

  void wrtTrmSt(TermStat* trmSt);
  void wrtCrpSt(CorpusStat* crpSt);
  void wrtTpcSt(TopicStat* tpcSt);
  void wrtTrmTpcF(TopicTermKey* trmTpcK, TopicTermValue* trmTpcV);
  void wrtEvalSt(EvaluationData* evSt); 
  void wrtStreamInfo(kba::term::StreamInfo strmInfo); 

  boost::shared_ptr<kba::term::TermStat> rdTrmStat(const char* term, time_t cPoint, bool seekStart, bool rstOffset);

  off_t getTrmStatOffset(time_t collTm, bool seekStart);
  off_t getCrpStatOffset(time_t collTm, bool seekStart);
  std::vector<boost::shared_ptr<EvaluationData> > getEvalData(std::string stream_id, bool seekStart, bool rstOffset);
  std::set<std::string> getEvalDocIds();
  boost::shared_ptr<kba::term::CorpusStat> rdCrpStat(time_t cPoint, bool seekStart, bool rstOffset);
  off_t  getStreamDbOffset(time_t sTime, bool seekStart);
  std::unordered_set<std::string> getDocIds(time_t sTime, time_t eTime, bool seekStart);
  void closeStreamDb();
  void closeTrmDb();
  void closeCrpDb();
};

inline void StatDb::wrtTrmSt(TermStat* trmSt) {
  if( _trmFd > 0) {
    size_t totalSize = sizeof(time_t) + (trmSt->term).size() + 1 + sizeof(long) + sizeof(long);
    char* packedData =  new char[totalSize+sizeof(size_t)];
    memset(packedData, 0, totalSize + sizeof(size_t));
    memcpy(packedData, &totalSize, totalSize);
    memcpy(packedData + sizeof(size_t), &(trmSt->collectionTime), sizeof(time_t));
    memcpy(packedData + sizeof(size_t)+ sizeof(time_t), (trmSt->term).c_str(), (trmSt->term).size() + 1);
    memcpy(packedData + sizeof(size_t)+ sizeof(time_t) + (trmSt->term).size() + 1, &(trmSt->docFreq), sizeof(long));
    memcpy(packedData + sizeof(size_t)+ sizeof(time_t) + (trmSt->term).size() + 1 + sizeof(long), &(trmSt->collFreq), sizeof(long));

  
    int status =  write(_trmFd, packedData, totalSize + sizeof(size_t)); 
    if (status < 0) {
      Logger::LOG_MSG("StatDb", "wrtTrmSt", "Fatal error ,Cannot write the Term Stat\n");
    }
    delete packedData;
  }  
}

inline void StatDb::wrtCrpSt(CorpusStat* crpSt) {
  if(_crpFd > 0) {
    size_t dataSize = sizeof(CorpusStat);
    char* packedData = new char[dataSize + sizeof(size_t)];
    memset(packedData, 0 , dataSize + sizeof(size_t));
    memcpy(packedData, &dataSize, sizeof(size_t));
    memcpy(packedData + sizeof(size_t), crpSt, sizeof(CorpusStat));
    int status = write(_crpFd, packedData, dataSize + sizeof(size_t));
    if(status < 0)
      Logger::LOG_MSG("StatDb", "wrtCrpSt", "Fatal error ,Cannot write the Corpus Stat\n");
    delete packedData;
  }
  else {
    Logger::LOG_MSG("StatDb", "wrtCrpSt", "Fatal error , Corpus stat db not open\n");
  }
}

#endif
