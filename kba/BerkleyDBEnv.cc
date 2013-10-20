#include "BerkleyDBEnv.hpp"
#include "stdexcept"
#include "Logging.hpp"

kba::berkley::CorpusDb::CorpusDb(std::string envBaseDir) {
  try {
    _dbEnv = new DbEnv(0);
    u_int32_t envFlags = DB_INIT_CDB |  DB_INIT_MPOOL;
    _dbEnv->open(envBaseDir.c_str(), envFlags, 0);
    _termDB = new Db(_dbEnv, 0);
    _termDB->open(NULL, "term.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE,0);
    _corpusDB = new Db(_dbEnv, 0);
    _corpusDB->open(NULL, "corpus.db", NULL, DB_BTREE, DB_AUTO_COMMIT | DB_CREATE, 0);
  } 
  catch(DbException &dbexpt) {
    _dbEnv = 0;
    _termDB = 0;
    _corpusDB = 0;
    Logger::LOG_MSG("BerkleyDBEnv.cc", "DBEnv", dbexpt.what());
  }
  catch(std::exception &expt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "DBEnv", expt.what());
  }
}


kba::berkley::CorpusDb::~CorpusDb() {
  try {
    if(_termDB != 0)
      _termDB->close(0);
    if(_corpusDB != 0) 
      _corpusDB->close(0);
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

void kba::berkley::CorpusDb::addTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value) {
  Dbt dbKey(key, sizeof(kba::term::TopicTermKey));
  Dbt dbValue(value, sizeof(kba::term::TopicTermValue));
  try {
    int status = CorpusDb::_termDB->put(NULL, &dbKey,&dbValue, DB_NOOVERWRITE);
    if (status == DB_KEYEXIST) {
      Logger::LOG_MSG("BerkleyDBEnv.cc", "addTerm", "key exist , not add to db");
    }
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "addTerm", dbexpt.what());
  }
  
} 

void kba::berkley::CorpusDb::updateCorpusStat(kba::term::CorpusStatKey* key, kba::term::CorpusStatValue* value) {
  Dbt dbKey(key, sizeof(kba::term::CorpusStatKey));
  Dbt dbValue(value, sizeof(kba::term::CorpusStatValue));
  try {
    int status = CorpusDb::_corpusDB->put(NULL, &dbKey, &dbValue, DB_NOOVERWRITE);
    if (status == DB_KEYEXIST) {
      Logger::LOG_MSG("BerkleyDBEnv.cc", "updateCorpusStat", "key exist , not added to db");
    }
  }
  catch (DbException &dbexpt) {
    Logger::LOG_MSG("BerkleyDBEnv.cc", "updateCorpusStat", dbexpt.what());
  }
  
}
