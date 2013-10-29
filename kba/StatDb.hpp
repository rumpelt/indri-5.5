#ifndef STATDB_HPP
#define STATDB_HPP
 
#include "stdio.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "TermDict.hpp"
#include "boost/shared_ptr.hpp"

using namespace kba::term;
class StatDb {
private:
  int _trmFd;
  int _trmDtWritten;
  
  int _crpFd;
  int _crpDtWritten;

  int _tpcFd;
  int _trmTpcFd;
  int _pgSz; // pagesize  
public:
  StatDb();
  ~StatDb();

  void crtTrmDb(std::string fName, bool rdOnly);
  void crtCrpDb(std::string fName, bool rdOnly);
  void crtTpcDb(std::string fName, bool rdOnly);
  void crtTrmTpcDb(std::string fName, bool rdOnly);

  void wrtTrmSt(TermStat* trmSt);
  void wrtCrpSt(CorpusStat* crpSt);
  void wrtTpcSt(TopicStat* tpcSt);
  void wrtTrmTpcF(TopicTermKey* trmTpcK, TopicTermValue* trmTpcV);
  
  boost::shared_ptr<kba::term::TermStat> rdTrmStat(const char* term, time_t cPoint, bool seekStart, bool rstOffset);
  off_t getTrmStatOffset(time_t collTm, bool seekStart);
  off_t getCrpStatOffset(time_t collTm, bool seekStart);
  boost::shared_ptr<kba::term::CorpusStat> rdCrpStat(time_t cPoint, bool seekStart, bool rstOffset);

};
#endif
