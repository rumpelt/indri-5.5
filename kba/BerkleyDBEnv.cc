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
  size_t keySize = (evalData->stream_id).size() + (evalData->topic).size() + 2;
  char* keyData = (char*) malloc(keySize);
  memset(keyData,0, keySize);
  memcpy(keyData,(evalData->stream_id).c_str(),  (evalData->stream_id).size() + 1);
  memcpy(keyData + (evalData->stream_id).size() + 1 ,(evalData->topic).c_str(),  (evalData->topic).size() + 1);

  DBT dbKey;
  memset(&dbKey, 0, sizeof(DBT));
  dbKey.data = keyData;
  dbKey.size = keySize;
  
  size_t dirSize = (evalData->directory).size() + 1;
  size_t assessorSize = (evalData->assessorId).size() + 1;
  size_t valueSize = dirSize + assessorSize + sizeof(int16_t) + sizeof(u_int16_t);
  char* valueData = new char[valueSize];
  memset(valueData, 0, valueSize);
  memcpy(valueData, &(evalData->rating), sizeof(int16_t));
  memcpy(valueData+sizeof(int16_t), &(evalData->cleanVisibleSize), sizeof(u_int16_t));
  memcpy(valueData+sizeof(int16_t)+sizeof(u_int16_t), (evalData->directory).c_str(), dirSize);
  memcpy(valueData+sizeof(int16_t)+sizeof(u_int16_t)+dirSize, (evalData->assessorId).c_str(), assessorSize);

  DBT dbValue;
  memset(&dbValue, 0, sizeof(DBT)); 
  dbValue.data = valueData;
  dbValue.size = valueSize;

  kba::berkley::CorpusDb::_evalDb->put(_evalDb, NULL, &dbKey, &dbValue, 0);

  delete keyData;
  delete valueData;
}

std::vector<boost::shared_ptr<kba::term::EvaluationData> > kba::berkley::CorpusDb::getEvaluationData(std::string stream_id, std::string topic) {
  using namespace kba::term;
  std::vector<boost::shared_ptr<kba::term::EvaluationData> > judgementData;

  size_t keySize = stream_id.size() + topic.size() + 2;
  char* keyData = (char*) malloc(keySize);
  memset(keyData,0, keySize);
  memcpy(keyData, stream_id.c_str(),  stream_id.size() + 1);
  memcpy(keyData + stream_id.size() + 1 , topic.c_str(),  topic.size() + 1);

  DBT dbKey;
  memset(&dbKey, 0, sizeof(DBT)); 
  dbKey.data = keyData;
  dbKey.size = keySize;
  
  DBT dbValue;
  memset(&dbValue, 0, sizeof(DBT));
  dbValue.flags = 0;

  DBC *cursor = 0;
  _evalDb->cursor(_evalDb, NULL,  &cursor, 0);
  int ret = cursor->get(cursor, &dbKey, &dbValue, DB_SET);
  while(ret != DB_NOTFOUND) {
    boost::shared_ptr<kba::term::EvaluationData> eval(new kba::term::EvaluationData());
    EvaluationData* edata = eval.get();
    edata->stream_id =  stream_id;
    edata->topic = topic;
    char* data = (char *)(dbValue.data);
    edata->rating = *((int16_t *)data);
    data = data + sizeof(int16_t);
    edata->cleanVisibleSize = (*(u_int16_t*)data);
    data = data + sizeof(u_int16_t);
    std::string directory(data); 
    edata->directory = directory;
    judgementData.push_back(eval);
    ret = cursor->get(cursor, &dbKey, &dbValue, DB_NEXT_DUP);
  }
  if(cursor != 0)
    cursor->close(cursor);
  
  delete keyData;  
  return judgementData;
}
