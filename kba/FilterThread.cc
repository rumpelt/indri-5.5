#include "FilterThread.hpp"
#include "DistributionImpl.hpp"
#include "lemur/Exception.hpp"
#include "PoissonModel.hpp"
#include "MultinomialDistribution.hpp"
#include "PoissonDistribution.hpp" 
#include "KLDivergenceModel.hpp" 
#include "ExpMxm.hpp"

std::fstream FilterThread::dumpStream;

FilterThread::FilterThread(indri::api::Parameters& params, std::string indexDir, std::map<std::string, query_t*> qMap, CorpusStat* corpusStat, std::map<std::string, TermStat*> termStatMap) :_dumpFile(""),_params(params), _indexDir(indexDir), _qMap(qMap), _corpusStat(corpusStat), _termStatMap(termStatMap), _docId(-1) {
  //_rep.openRead(indexDir);
  //  _qServer = new indri::server::LoclQuerServer(_rep);
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


void FilterThread::dumpKbaResult(std::string& queryId, std::vector<ResultStruct>& resultPool, std::string &dumpFile) {
  std::fstream dumpStream(dumpFile.c_str(), std::fstream::out | std::fstream::app);
  std::string teamId = "udel";
  for(std::vector<ResultStruct>::iterator rsIt = resultPool.begin(); rsIt != resultPool.end(); ++rsIt ) {
    ResultStruct rs  = *rsIt;
    dumpStream << teamId << " " << FilterThread::_runId << " " << rs.id << " " << queryId << " " << rs.score <<  " " << "2" << " " << "1" << " " << rs.dayDt << " " << "NULL" << " " << "-1" << " " <<"0-0" <<  "  " << rs.origScore << "\n";
          //    std::cout << rs.score << "\n";
  }
  dumpStream.close();
}

void FilterThread::dumpKbaResult(std::string& queryId, std::priority_queue<ResultStruct, std::vector<ResultStruct>, ResultStruct::lesser>& resultPool, std::string &dumpFile, int retainCount) {
  //std::fstream dumpStream(dumpFile.c_str(), std::fstream::out | std::fstream::app);
  std::string teamId = "udel";
  int count = 0;
  while(!resultPool.empty() && (count < retainCount)) {
    ResultStruct rs  = resultPool.top();
    resultPool.pop();
    dumpStream << teamId << " " << FilterThread::_runId << " " << rs.id << " " << queryId << " " << 1000 << " " << "2" << " " << "1" << " " << rs.dayDt << " " << "NULL" << " " << "-1" << " " <<"0-0" <<  "  " << rs.score  << " " << rs.indriScore << "\n";
    //    std::cout << rs.score << "\n";
    count++;
  }
  //dumpStream.close();
}


void FilterThread::dumpKbaResult(std::string& queryId, std::priority_queue<ResultStruct, std::vector<ResultStruct>, ResultStruct::greater>& resultPool, std::string& dumpFile, int retainCount) {
  //std::fstream dumpStream(dumpFile.c_str(), std::fstream::out | std::fstream::app);
  std::string teamId = "udel";
  int count = 0;
  while(!resultPool.empty() && (count < retainCount)) {
    ResultStruct rs  = resultPool.top();
    resultPool.pop();
    dumpStream << teamId << " " << FilterThread::_runId << " " << rs.id << " " << queryId << " " << 1000 << " " << "2" << " " << "1" << " " << rs.dayDt << " " << "NULL" << " " << "-1" << " " <<"0-0" <<  "  " << rs.score << " " << rs.indriScore <<"\n";
    //    std::cout << rs.score << "\n";
    count++;
  }
  //dumpStream.close();
}

void FilterThread::indriBasicRun(query_t* query) {
  //std::cout << "Scoring dir " << _indexDir  << " query "<< query->text  << "type " << query->qType << std::endl;
  QueryThread qt(FilterThread::_params, FilterThread::_indexDir);
  qt._runQuery(query->text, query->qType);
  std::string kbaIdKey = "docno";
  std::vector<std::string> kbaIds = qt.getMetadata(kbaIdKey);
  std::vector<ResultStruct> results;
  int index = 0;
  int count = 0;
  for(std::vector<std::string>::iterator docIt = kbaIds.begin(); docIt != kbaIds.end() && count < 500; ++docIt, ++index, ++count) {
    std::string docId = *docIt;
    ResultStruct rs(0);
    rs.id = docId;
    rs.dayDt = FilterThread::_indexDir;
    rs.score = 1000;
    rs.origScore = qt.getScore(index);
    results.push_back(rs);
  }
  dumpKbaResult(query->id, results, FilterThread::_dumpFile);
}

Distribution* FilterThread::createDistribution(QueryThread& oldQt, std::vector<std::string>& textVector, double mu) {
  std::map<std::string, unsigned long> termFreqMap;
  std::map<std::string, double> collectionProb;
  unsigned long collSize = oldQt.termCount();
  for(std::vector<std::string>::const_iterator textIt = textVector.begin(); textIt != textVector.end(); ++textIt) {
    std::string term = *textIt;
    termFreqMap[term]++; 
  }
  for(std::map<std::string, unsigned long>::const_iterator textIt = termFreqMap.begin(); textIt != termFreqMap.end(); ++textIt) {
    std::string term = textIt->first;
    double termCount = oldQt.termCount(term);
    if(termCount <= 0)
      termCount = 1;
    collectionProb[term] = termCount / collSize;
  }
  //PoissonDistribution* pd = new PoissonDistribution(mu);
  //pd->initialize(termFreqMap, collectionProb, textVector.size());
  //return pd;
  MultinomialDistribution* md = new MultinomialDistribution(mu);
  md->initialize(termFreqMap, collectionProb, textVector.size());
  return md;
}

void FilterThread::updateModel(QueryThread& oldQt, Model* model) {
  model->setCollectionSize(oldQt.termCount());
  //  std::cout << "Collection size " << oldQt.termCount() << std::endl;
  for(std::map<std::string, query_t*>::iterator qMapIt = _qMap.begin(); qMapIt != _qMap.end(); ++qMapIt) {
 
    std::string qId = qMapIt->first;
    
    query_t* query = qMapIt->second;
    //    std::cout << "Updateing for query " << query->text << std::endl;
    for(std::vector<std::string>::iterator vecIt = (query->textVector).begin(); vecIt!= (query->textVector).end(); ++vecIt) {
      std::string term = *vecIt;
      //  std::cout << "Term Count " << term << oldQt.termCount(term) << std::endl;
      model->setCollectionFreq(term, oldQt.termCount(term));
    }
    //query->distribution = FilterThread::createDistribution(oldQt, query->textVector, 0);
  }
}

void FilterThread::dumpDayStat(QueryThread& oldQt) {
  QueryThread qt(FilterThread::_params, FilterThread::_indexDir);
  // std::cout << qt.documentCount() << " " <<  oldQt.documentCount() << std::endl;
  double todayDocSize = 0.0;
  if (qt.documentCount() > 0.0) 
    todayDocSize = qt.termCount() / qt.documentCount();
  double oldDocSize = oldQt.termCount() / oldQt.documentCount();
  //FilterThread::dumpStream << FilterThread::_indexDir << "," << "present-day," << todayDocSize << "\n";
  FilterThread::dumpStream << FilterThread::_indexDir << "," << "past-5-day," << oldDocSize << "\n";
}

void FilterThread::scoreAndDump(std::string queryId, query_t* query, Model& model, QueryThread& oldQt) {
  //  std::cout << "Scoring dir " << _indexDir << std::endl;
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
  std::cout << "Query :";
  for(std::vector<std::string>::iterator vecIt = (query->textVector).begin(); vecIt!= (query->textVector).end(); ++vecIt)
    std::cout << *vecIt << " ";
  std::cout << std::endl;
  */

  std::vector<lemur::api::DOCID_T> docIds = qt.getDocIds();
  for(std::vector<indri::api::DocumentVector*>::iterator dvsIt = dvs.begin(); dvsIt != dvs.end() ; ++dvsIt, ++idx) {
    //    std::cout << "Indri score " << qt.getScore(idx) << std::endl;
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
    //Distribution* psgDist = FilterThread::createDistribution(oldQt, docContent, 2500);
 
    float score = model.score(query->textVector, &psg);
    //float score = model.score(*query, *psgDist); // thisis for KL DivergeneModel
    //delete psgDist;

    ResultStruct rs(0);
    rs.origScore = 1000;
    rs.indriScore = (int)(qt.getScore(idx));
    rs.id = kbaIds[idx];
    rs.dayDt = FilterThread::_indexDir;
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


void FilterThread::expectationMaximDistribution(QueryThread& oldQt, Model* model) {
  model->setCollectionSize(oldQt.termCount());
  for(std::map<std::string, query_t*>::iterator qMapIt = _qMap.begin(); qMapIt != _qMap.end(); ++qMapIt){
    std::string qId = qMapIt->first;
    query_t* query = qMapIt->second;

    std::vector<std::string> filteredVec;
    unsigned long collSize = oldQt.termCount();
    std::map<std::string, double> collProb;

    for(std::vector<std::string>::const_iterator textIt = query->textVector.begin(); textIt != query->textVector.end(); ++textIt) {
      std::string term = *textIt;
      double gcount = oldQt.termCount(term);
     
      double prob = gcount / collSize;
      
      if(prob <= 0.000000001)
        continue;
      model->setCollectionFreq(term, gcount);
      filteredVec.push_back(term);
      collProb[term] = prob;
    }

    std::map<std::string, double> termProb;
    expmxm::iteration(qId, filteredVec, collProb, termProb);
    DistributionImpl* dimpl = new DistributionImpl();
    dimpl->setTermProb(termProb);
    dimpl->setCollectionProb(collProb);
    query->distribution = dimpl;
  } 
}

void FilterThread::process(QueryThread& oldQt) {
  LanguageModelPsg pmodel(5000);
  //PoissonModel pmodel(5000);
  //KLDivergenceModel pmodel;
  FilterThread::updateModel(oldQt, &pmodel); 
  //FilterThread::expectationMaximDistribution(oldQt,&pmodel);
  
  //dumpStream.open(FilterThread::_dumpFile.c_str(), std::fstream::out | std::fstream::app);
  for(std::map<std::string, query_t*>::iterator qMapIt = _qMap.begin(); qMapIt != _qMap.end(); ++qMapIt) {
    std::string qId = qMapIt->first;
    query_t* q = qMapIt->second;
    FilterThread::scoreAndDump(qId, q, pmodel, oldQt);
    //FilterThread::indriBasicRun(q);
    //delete q->distribution;
  }
  //  dumpStream.close();
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
