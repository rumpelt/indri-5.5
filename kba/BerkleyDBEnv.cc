#include "BerkleyDBEnv.hpp"
#include "stdexcept"
#include "Logging.hpp"
#include <string.h>
#include <assert.h>

kba::berkley::CorpusDb::CorpusDb(std::string envBaseDir, bool readOnly) {
  db_env_create(&_dbEnv, 0);
  u_int32_t envFlags = DB_CREATE |  DB_INIT_MPOOL | DB_THREAD;
  _dbEnv->open(_dbEnv, envBaseDir.c_str(), envFlags, 0);
  u_int32_t dbFlags;
  if(readOnly)
    dbFlags = DB_RDONLY;
  else
    dbFlags = DB_CREATE | DB_EXCL;
  db_create(&_termTopicDb,_dbEnv, 0);
  _termTopicDb->open(_termTopicDb, NULL, "term-topic.db", NULL, DB_BTREE, dbFlags, 0);

  db_create(&_topicDb,_dbEnv, 0);
  _topicDb->open(_topicDb, NULL, "topic.db", NULL, DB_BTREE, dbFlags,0);
    
  db_create(&_termDb,_dbEnv, 0);
  _termDb->open(_termDb, NULL, "term.db", NULL, DB_BTREE, dbFlags, 0);
  
  db_create(&_corpusDb,_dbEnv, 0);
  _corpusDb->open(_corpusDb, NULL, "corpus.db", NULL, DB_BTREE, dbFlags, 0);

  db_create(&_evalDb, _dbEnv, 0);
  _evalDb->set_flags(_evalDb, DB_DUP);
  _evalDb->open(_evalDb, NULL, "eval.db", NULL, DB_BTREE, DB_RDONLY, 0);
}

void kba::berkley::CorpusDb::init(std::string envBaseDir, std::string evalDbName) {
  _dbEnv = 0;

  int status = db_env_create(&_dbEnv, 0);
  if (status != 0)
    std::cout << "Error creating Env: "<<db_strerror(status) << "\n";

  u_int32_t envFlags = (DB_CREATE |DB_INIT_MPOOL | 1);
  const char* dirName = envBaseDir.c_str();
  status = (*_dbEnv).open(_dbEnv, dirName, envFlags, 0);
  if (status != 0) {
    std::cout  << "Error opening Env: "<<db_strerror(status) << " Env Dir " << envBaseDir.c_str() <<  db_version(NULL, NULL, NULL) <<  "\n";

  }

  status = db_create(&_evalDb, NULL, 0);
  if (status != 0)
    std::cout << db_strerror(status) << "\n";
  u_int32_t dbFlag = DB_DUPSORT;
  _evalDb->set_flags(_evalDb, dbFlag);
  _evalDb->open(_evalDb, NULL, (envBaseDir + evalDbName).c_str(), NULL, DB_BTREE, DB_CREATE, 0); 
}

kba::berkley::CorpusDb::CorpusDb(std::string envBaseDir, std::string evalDbName) : _dbEnv(0), _termDb(0), _termTopicDb(0), _corpusDb(0), _topicDb(0), _evalDb(0){
  
}
   


kba::berkley::CorpusDb::~CorpusDb() {
  if(_termDb != 0)
    _termDb->close(_termDb, 0);
  if(_topicDb != 0)
    _topicDb->close(_topicDb, 0);
  if(_termTopicDb != 0)
    _termTopicDb->close(_termTopicDb, 0);
  if(_corpusDb != 0) 
    _corpusDb->close(_corpusDb, 0);
  _evalDb->close(_evalDb, 0);
  _dbEnv->close(_dbEnv, 0);
}

void kba::berkley::CorpusDb::addTopicTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value) {
  using namespace kba::berkley;
  std::string topic(key->topic);
  std::string term(key->term);
  size_t totalSize = sizeof(time_t) + topic.size() + term.size()+2;
  char* packedData =(char*) malloc(totalSize);
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(key->collectionTime), sizeof(time_t));
  memcpy(packedData+sizeof(time_t), (key->term).c_str(), term.size() + 1 );
  memcpy(packedData+sizeof(time_t)+term.size() + 1, (key->topic).c_str(), topic.size() + 1 );

  DBT dbKey, dbValue;
  memset(&dbKey,0,sizeof(DBT));
  memset(&dbValue,0,sizeof(DBT));
  dbKey.data = packedData;
  dbKey.size = totalSize;
  dbValue.data = value;
  dbValue.size = sizeof(kba::term::TopicTermValue);
  int status = CorpusDb::_termDb->put(_termDb, NULL, &dbKey, &dbValue, DB_NOOVERWRITE);
  if (status == DB_KEYEXIST) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addTerm", "key exist , not add to db");
  }
  delete packedData;
}


void kba::berkley::CorpusDb::addCorpusStat(kba::term::CorpusStat* corpusStat) {
  DBT dbKey, dbValue;
  memset(&dbKey, 0, sizeof(DBT));
  memset(&dbValue, 0, sizeof(DBT));
  dbKey.data = &(corpusStat->collectionTime);
  dbKey.size = sizeof(time_t);
  dbValue.data = corpusStat;
  dbValue.size = sizeof(kba::term::CorpusStat);

  int status = kba::berkley::CorpusDb::_corpusDb->put(_corpusDb, NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
  if(status == DB_KEYEXIST) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addCorpusStat", "DB key exist");
  }
}

void kba::berkley::CorpusDb::addTermStat(kba::term::TermStat* termStat) {
  size_t totalSize = sizeof(time_t) + (termStat->term).size() + 1;
  char* packedData = (char*) malloc(totalSize);
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(termStat->collectionTime), sizeof(time_t));
  memcpy(packedData + sizeof(time_t), (termStat->term).c_str(), (termStat->term).size() + 1);

  DBT dbKey;
  memset(&dbKey, 0, sizeof(DBT));
  dbKey.data = packedData;
  dbKey.size = totalSize;

  DBT dbValue;
  memset(&dbValue, 0, sizeof(DBT));
  dbValue.data = &(termStat->docFreq);
  dbValue.size = sizeof(unsigned int);
 
  int status = kba::berkley::CorpusDb::_termDb->put(_termDb, NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
  if(status == DB_KEYEXIST) {
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTermStat", "Key already exist");
  }
  delete packedData;  
}

void kba::berkley::CorpusDb::addTopicStat(kba::term::TopicStat* topicStat) {

  size_t totalSize = sizeof(time_t) + (topicStat->topic).size() + 1;
  char* packedData = (char *) malloc(totalSize);
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(topicStat->collectionTime), sizeof(time_t));
  memcpy(packedData + sizeof(time_t), (topicStat->topic).c_str(), (topicStat->topic).size() + 1);

  DBT dbKey;
  memset(&dbKey, 0, sizeof(DBT));
  dbKey.data = packedData;
  dbKey.size = totalSize;
 
  DBT dbValue;
  memset(&dbValue, 0, sizeof(DBT));
  dbValue.data = &(topicStat->relevantSetSize);
  dbValue.size = sizeof(int);
 
  int status = kba::berkley::CorpusDb::_topicDb->put(_topicDb, NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
  if(status == DB_KEYEXIST) {
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTopicStat", "Key already exist");
  }
  delete packedData;  
}

void kba::berkley::CorpusDb::addEvaluationData(kba::term::EvaluationData* evalData) {
 
}

std::vector<boost::shared_ptr<kba::term::EvaluationData> > kba::berkley::CorpusDb::getEvaluationData(std::string stream_id, std::string topic) {
  std::vector<boost::shared_ptr<kba::term::EvaluationData> >ev;
  return ev;
}
