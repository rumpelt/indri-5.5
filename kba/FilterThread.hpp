#ifndef FILTERTHREAD_HPP
#define FILTERTHREAD_HPP

#include "ResultPool.hpp"
#include "QueryThread.hpp"
#include "PassageModel.hpp"
#include "LanguageModelPsg.hpp"
#include "TermDict.hpp"
#include "indri/Repository.hpp"
#include "indri/LocalQueryServer.hpp"
#include "indri/ScoredExtentResult.hpp"
#include "indri/QueryEnvironment.hpp"
#include <map>
#include <queue>
#include <vector>

using namespace kba::term;
/**
 * this is currently for single indexes only.
 */
class FilterThread {
private:
  std::string _indexDir;
  indri::collection::Repository _rep;
  indri::server::LocalQueryServer* _qServer;
  CorpusStat* _corpusStat;
  std::map<std::string, TermStat*> _termStatMap;
  
  lemur::api::DOCID_T _docId; // to be used to to fetch document vector. This should be set to next docId or -1.
  lemur::api::DOCID_T _maxDocId; // to be set using indri::index::Index api
  std::map<std::string, query_t*> _qMap;
  QueryThread* _qt;
  void setDocId();
  std::vector<lemur::api::DOCID_T> docIdSet(int numIds); // get a bunch of docIds. incrementatl in nature. Uses _docId field above.
  std::vector<indri::api::DocumentVector*> documentVector(int numDocs);
  void freeDocVectors(std::vector<indri::api::DocumentVector*> docs);
  void scoreDocVector(indri::api::DocumentVector* doc);

public:
  std::string _dumpFile;
  std::string _runId;
  indri::api::Parameters& _params;
  FilterThread(indri::api::Parameters& param, std::string indexDir, std::map<std::string, query_t*> qMap, CorpusStat* corpusStat, std::map<std::string, TermStat*> termStatMap);

  ~FilterThread();
  void initialize();
  void update();
  void   process();
  float score(indri::api::ScoredExtentResult& sr, LanguageModelPsg& lmpsg);
  void  scoreAndDump(std::string queryId, query_t* query, LanguageModelPsg& lm);
  void setParamFile(std::vector<std::string> paramFiles);
  void dumpKbaResult(std::string& queryId, std::priority_queue<ResultStruct, std::vector<ResultStruct>, ResultStruct::greater>& resultPool, std::string& dumpFile, int retainCount);
};
#endif
