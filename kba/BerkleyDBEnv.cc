#include "BerkleyDBEnv.hpp"
#include "stdexcept"
#include "Logging.hpp"
#include <string.h>


kba::berkley::CorpusDb::CorpusDb(std::string envBaseDir) {
  try {
    _dbEnv = new DbEnv(0);
    u_int32_t envFlags = DB_INIT_CDB |  DB_INIT_MPOOL;
    _dbEnv->open(envBaseDir.c_str(), envFlags, 0);
    _termTopicDb = new Db(_dbEnv, 0);
    _termTopicDb->open(NULL, "term-topic.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE,0);

    _topicDb = new Db(_dbEnv, 0);
    _topicDb->open(NULL, "topic.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE,0);
   
    _termDb = new Db(_dbEnv, 0);
    _termDb->open(NULL, "term.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE,0);

    _corpusDb = new Db(_dbEnv, 0);
    _corpusDb->open(NULL, "corpus.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE, 0);
  } 
  catch(DbException &dbexpt) {
    _dbEnv = 0;
    _termDb = 0;
    _corpusDb = 0;
    Logger::LOG_MSG("BerkleyDBEnv.cc", "DBEnv", dbexpt.what());
  }
  catch(std::exception &expt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "DBEnv", expt.what());
  }
}


kba::berkley::CorpusDb::~CorpusDb() {
  try {
    if(_termDb != 0)
      _termDb->close(0);
    if(_topicDb != 0)
      _topicDb->close(0);
    if(_termTopicDb != 0)
       _termTopicDb->close(0);
    if(_corpusDb != 0) 
      _corpusDb->close(0);
    if(_dbEnv != 0)
      _dbEnv->close(0);
  }
  catch(DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "~CorpusDb", dbexpt.what());
  }
  catch(std::exception &expt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "~CorpusDb", expt.what());
  }
}

void kba::berkley::CorpusDb::addTopicTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value) {
  using namespace kba::berkley;
  std::string topic(key->topic);
  std::string term(key->term);
  size_t totalSize = sizeof(time_t) + topic.size() + term.size()+2;
  char* packedData = new char[totalSize];
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(key->collectionTime), sizeof(time_t));
  memcpy(packedData+sizeof(time_t), (key->term).c_str(), term.size() + 1 );
  memcpy(packedData+sizeof(time_t)+term.size() + 1, (key->topic).c_str(), topic.size() + 1 );

  Dbt dbKey(packedData, totalSize);
  Dbt dbValue(value, sizeof(kba::term::TopicTermValue));

  try {
    int status = CorpusDb::_termDb->put(NULL, &dbKey,&dbValue, DB_NOOVERWRITE);
    if (status == DB_KEYEXIST) {
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTerm", "key exist , not add to db");
    }
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addTerm", dbexpt.what());
  }

  delete packedData;
} 


void kba::berkley::CorpusDb::addCorpusStat(kba::term::CorpusStat* corpusStat) {
  Dbt dbKey(&(corpusStat->collectionTime), sizeof(time_t));
  Dbt dbValue(corpusStat, sizeof(kba::term::CorpusStat));
  try {
    int status = kba::berkley::CorpusDb::_corpusDb->put(NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
    if(status == DB_KEYEXIST)
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addCorpusStat", "DB key exist");
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addCorpusStat", dbexpt.what());
  }
}

void kba::berkley::CorpusDb::addTermStat(kba::term::TermStat* termStat) {
  size_t totalSize = sizeof(time_t) + (termStat->term).size() + 1;
  char* packedData = new char[totalSize];
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(termStat->collectionTime), sizeof(time_t));
  memcpy(packedData + sizeof(time_t), (termStat->term).c_str(), (termStat->term).size() + 1);
  Dbt dbKey(packedData, totalSize);
  Dbt dbValue(&(termStat->docFreq), sizeof(unsigned int)); 
  try {
    int status = kba::berkley::CorpusDb::_termDb->put(NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
    if(status == DB_KEYEXIST)
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTermStat", "Key already exist");
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addTermStat", dbexpt.what());
  }

  delete packedData;  
}

void kba::berkley::CorpusDb::addTopicStat(kba::term::TopicStat* topicStat) {

  size_t totalSize = sizeof(time_t) + (topicStat->topic).size() + 1;
  char* packedData = new char[totalSize];
  memset(packedData, 0, totalSize);
  memcpy(packedData, &(topicStat->collectionTime), sizeof(time_t));
  memcpy(packedData + sizeof(time_t), (topicStat->topic).c_str(), (topicStat->topic).size() + 1);
  Dbt dbKey(packedData, totalSize);
  Dbt dbValue(&(topicStat->relevantSetSize), sizeof(int)); 
  try {
    int status = kba::berkley::CorpusDb::_topicDb->put(NULL, &dbKey, &dbValue,DB_NOOVERWRITE);
    if(status == DB_KEYEXIST)
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTopicStat", "Key already exist");
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addTopicStat", dbexpt.what());
  }

  delete packedData;  
}
