#include "FilterThread.hpp"
#include "lemur/Exception.hpp"

FilterThread::FilterThread(indri::api::Parameters& params, std::string indexDir, std::map<std::string, query_t*> qMap, CorpusStat* corpusStat, std::map<std::string, TermStat*> termStatMap) :_dumpFile(""),_params(params), _indexDir(indexDir), _qMap(qMap), _corpusStat(corpusStat), _termStatMap(termStatMap), _docId(-1) {
  //_rep.openRead(indexDir);
  //  _qServer = new indri::server::LocalQuerServer(_rep);
}

/**
 * Set the base doc id so that we can iterate on docs in a index
 */
void FilterThread::setDocId() {
  indri::collection::Repository::index_state state = FilterThread::_rep.indexes();
  if(state->size() > 0) {
    indri::index::Index* index = (*state)[0];
    FilterThread::_docId = index->documentBase();
    FilterThread::_maxDocId = index->documentMaximum();
  }
  else {
    LEMUR_THROW( LEMUR_RUNTIME_ERROR, "No index assocated with repostory" );
  }
}

std::vector<lemur::api::DOCID_T> FilterThread::docIdSet(int numIds) {
  if(FilterThread::_docId  <= 0)
    FilterThread::setDocId();
  std::vector<lemur::api::DOCID_T> docIds;
  int count = 0;
  for(lemur::api::DOCID_T curId = FilterThread::_docId ; curId < FilterThread::_maxDocId && count < numIds; ++curId, ++count) {
    docIds.push_back(curId);
  }
  FilterThread::_docId += docIds.size(); // Set the _docId to correct position.
  return docIds;
}

std::vector<indri::api::DocumentVector*> FilterThread::documentVector(int numDocs) {
  std::vector<lemur::api::DOCID_T> docIds = FilterThread::docIdSet(numDocs);
  indri::server::QueryServerVectorsResponse* response = _qServer->documentVectors(docIds);
  std::vector<indri::api::DocumentVector*> docs = response->getResults();
  delete response;
  return docs;
}

void FilterThread::freeDocVectors(std::vector<indri::api::DocumentVector*> docs) {
  for(std::vector<indri::api::DocumentVector*>::iterator docIt = docs.begin(); docIt != docs.end(); ++docIt) {
    delete *docIt;
  }
  docs.clear();
}


void FilterThread::dumpKbaResult(std::string& queryId, std::priority_queue<ResultStruct, std::vector<ResultStruct>, ResultStruct::greater>& resultPool, std::string &dumpFile, int retainCount) {
  std::fstream dumpStream(dumpFile.c_str(), std::fstream::out | std::fstream::app);
  std::string teamId = "udel";
  int count = 0;
  while(!resultPool.empty() && (count < retainCount)) {
    ResultStruct rs  = resultPool.top();
    resultPool.pop();
    dumpStream << teamId << " " << FilterThread::_runId << " " << rs.id << " " << queryId << " " << "1000" << " " << "2" << " " << "1" << " " << rs.dayDt << " " << "NULL" << " " << "-1" << " " <<"0-0" <<  "  " << rs.score << "\n";
    //    std::cout << rs.score << "\n";
    count++;
  }
  dumpStream.close();
}

void FilterThread::scoreAndDump(std::string queryId, query_t* query, LanguageModelPsg& lm) {
  std::cout << "Scoring dir " << _indexDir << std::endl;
  QueryThread qt(FilterThread::_params, FilterThread::_indexDir);
  qt._runQuery(query, false, 10000, 10000);
  std::string kbaIdKey = "docno";
  std::vector<std::string> kbaIds = qt.getMetadata(kbaIdKey);
  //if (kbaIds.size() <= 0)
  // return;
  std::vector<indri::api::DocumentVector*> dvs = qt.getDocumentVector();
  
  std::priority_queue<ResultStruct, std::vector<ResultStruct>, ResultStruct::greater> resultPool;
  int idx=0;
  
  /**
  kbaIdKey = "time";
  std::vector<std::string> timeIds = qt.getMetadata(kbaIdKey);
  kbaIdKey = "file";
  std::vector<std::string> fileIds = qt.getMetadata(kbaIdKey);
  std::string metaDirKey = "dir";
  std::vector<std::string> dirs = qt.getMetadata(metaDirKey);
  
  std::cout << "Query :";
  for(std::vector<std::string>::iterator vecIt = (query->textVector).begin(); vecIt!= (query->textVector).end(); ++vecIt)
    std::cout << *vecIt << " ";
  std::cout << std::endl;
  */
  std::vector<lemur::api::DOCID_T> docIds = qt.getDocIds();
  for(std::vector<indri::api::DocumentVector*>::iterator dvsIt = dvs.begin(); dvsIt != dvs.end() ; ++dvsIt, ++idx) {
    indri::api::DocumentVector* dv = *dvsIt;
    std::vector<std::string> docContent = PassageModel::constructDocFromVector(dv, true);
    /**
    std::cout << "Document :";
    for(std::vector<std::string>::iterator vecIt = docContent.begin(); vecIt!= docContent.end(); ++vecIt)
      std::cout << *vecIt << " ";
    std::cout << std::endl;
    */
    Passage psg = PassageModel::createPassage(docContent, docIds[idx], true);
    psg.crtTermFreq();
    
    float score = lm.score(query->textVector, &psg, FilterThread::_termStatMap, FilterThread::_corpusStat);
    ResultStruct rs(0);
    rs.id = kbaIds[idx];
    rs.dayDt = "2012-10-00";
    rs.score = (int) score;
    resultPool.push(rs);
  }

  FilterThread::dumpKbaResult(queryId, resultPool, FilterThread::_dumpFile, 100);
  FilterThread::freeDocVectors(dvs);  

}

void FilterThread::setParamFile(std::vector<std::string> paramFiles) {
  _params = indri::api::Parameters::instance();
  for(std::vector<std::string>::iterator paramIt = paramFiles.begin(); paramIt != paramFiles.end(); ++paramIt) {
    FILE* pFile = fopen((*paramIt).c_str(), "rb");
    if(pFile != NULL) {


      //      std::cout << "Loading file " << *paramIt << "\n";
      _params.loadFile(*paramIt);
      fclose(pFile);
    }
  }
  
}

void FilterThread::scoreDocVector(indri::api::DocumentVector* doc) {
  
}

void FilterThread::process() {
  LanguageModelPsg lm;
  for(std::map<std::string, query_t*>::iterator qMapIt = _qMap.begin(); qMapIt != _qMap.end(); ++qMapIt) {
    std::string qId = qMapIt->first;
    query_t* q = qMapIt->second;
    FilterThread::scoreAndDump(qId, q, lm);
  }
  FilterThread::update();
}

void FilterThread::update() {
  QueryThread qt(FilterThread::_params, FilterThread::_indexDir);
  for(std::map<std::string, TermStat*>::iterator termIt = _termStatMap.begin(); termIt != _termStatMap.end(); termIt++ ) {
    std::string term = termIt->first;
    TermStat* termStat = termIt->second;
    termStat->docFreq += qt.documentCount(term);
    termStat->collFreq += qt.termCount(term);
  }

  FilterThread::_corpusStat->totalDocs += qt.documentCount();
  FilterThread::_corpusStat->collectionSize += qt.termCount();
}

FilterThread::~FilterThread() {
}
