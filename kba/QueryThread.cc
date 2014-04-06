#include "QueryThread.hpp"
#include <dirent.h>

QueryThread::QueryThread(indri::api::Parameters& params, std::vector<std::string> dirs) : 
  _parameters(params), _expander(0), _indexDirs(dirs){
  initialize();
}

QueryThread::QueryThread(indri::api::Parameters& params, std::string dir) : 
  _parameters(params), _expander(0){
  _indexDirs.push_back(dir);
  initialize();
}

UINT64 QueryThread::initialize() {

  _environment.setSingleBackgroundModel( QueryThread::_parameters.get("singleBackgroundModel", false)); //  Dont what is this for.
  
  std::vector<std::string> stopwords;
  if( copy_parameters_to_string_vector( stopwords, _parameters, "stopper.word" ))
    _environment.setStopwords(stopwords);
  
  std::vector<std::string> smoothingRules;
  if( copy_parameters_to_string_vector( smoothingRules, _parameters, "rule" ) ) // Smothing ruls to be used refer IndriRunQuery documentation.
    _environment.setScoringRules( smoothingRules );

  for(std::vector<std::string>::iterator dirIt = _indexDirs.begin(); dirIt != _indexDirs.end(); ++dirIt) {
    DIR* dirc = opendir((*dirIt).c_str());
    if(dirc != NULL) {
      _environment.addIndex(*dirIt); // Add the indexes
      closedir(dirc);
    }
    else {
      std::cout << "could not open the index dir " << *dirIt << "\n";
    }
  }
  
  if( _parameters.exists("maxWildcardTerms") )
    _environment.setMaxWildcardTerms(_parameters.get("maxWildcardTerms", 100)); // Number of wildcardTerms to be generated

  _requested = _parameters.get( "count", 1000 ); // The number of results requested
  _initialRequested = _parameters.get( "fbDocs", _requested ); // If pesudo feedback specified then number of initial results requested
  _runId = _parameters.get( "runID", "kba" );  // The run Id...default is kba
  

  if (_parameters.exists("baseline")) { // if we requested a baseline run
      // doing a baseline
      std::string baseline = _parameters["baseline"];
      _environment.setBaseline(baseline);
      // need a factory for this...
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        // have to push the method in...
        std::string rule = "method:" + baseline;
        _parameters.set("rule", rule);
        _expander = new indri::query::TFIDFExpander( &_environment, _parameters );
      }
    } else {
      if( _parameters.get( "fbDocs", 0 ) != 0 ) {
        _expander = new indri::query::RMExpander( &_environment, _parameters );
      }
    }
  return 0;
}

void QueryThread::addIndexDir(std::string dir) {
  DIR* dirc = opendir(dir.c_str());
  if(dirc != NULL) {
    //std::cout << "Adding dir " << dir << std::endl; 
    _environment.addIndex(dir); // Add the indexes
    closedir(dirc);
  }
  else {
    std::cout << "could not open the index dir " << dir << "\n";
  }
}

void QueryThread::removeDir(std::string dir) {
  DIR* dirc = opendir(dir.c_str());
  if(dirc != NULL) {
    //   std::cout << "Removind index " << dir << std::endl;
    _environment.removeIndex(dir); // Add the indexes
    closedir(dirc);
  }
  else {
    std::cout << "could not remove the index dir " << dir << "\n";
  }
}

void QueryThread::_runQuery(std::string queryText ,std::string queryType) {
  try {
    //    std::cout << "Executing Quyery " << queryText << std::endl;  

    _results = _environment.runQuery( queryText, _initialRequested, queryType ); //Runs query
    if(_expander) {
      std::string expandedQuery;
      expandedQuery = _expander->expand( queryText, _results);
      //      std::cout << "Expanding query" << std::endl;
      _results = _environment.runQuery(expandedQuery, _requested, queryType );
    }
    //    std::cout << "Num results " << _results.size() << "\n";
  }
  catch(lemur::api::Exception& e) {
   _results.clear();
   LEMUR_RETHROW(e, "QueryThread::_runQuery Exception");
  }
}

void QueryThread::_runQuery(query_t* q, bool useFeedBack, int initRequestSize, int requestSize) {
  indri::query::QueryExpander*  localExpander=0;
  try {
    // std::cout << q->text << std::endl;
    _results = _environment.runQuery(q->text, initRequestSize, q->qType);
    //    std::cout << "Init result " << _results.size() << "\n";
    if(useFeedBack) {
      localExpander = new indri::query::RMExpander(&_environment, _parameters);
      q->expandedText = localExpander->expand(q->text, _results);
      _results = _environment.runQuery(q->expandedText, requestSize, q->qType);
      //std::cout << "Expanded Result size " << _results.size() << "\n";
      delete localExpander;    
    }
  }
  catch(lemur::api::Exception& e) {
    if(localExpander != 0) {
      delete localExpander;
    }
    LEMUR_RETHROW(e, "QueryThread::_runQuery(query_t) Exception"); 
  }
}




void QueryThread::dumpKbaResult(std::vector<indri::api::ScoredExtentResult>& results, std::string queryId, std::string dumpfile) {
  std::string docno = "docno";
  std::vector<std::string> trecIds = _environment.documentMetadata(results, docno);
  std::string dir = "dir";
  std::vector<std::string> dirs = _environment.documentMetadata(results, dir);

  std::string teamId = "udel";
  
  std::fstream dumpStream(dumpfile.c_str(), std::fstream::out | std::fstream::app);
  
  int idx = 0;
  for (std::vector<indri::api::ScoredExtentResult>::iterator srIt = results.begin(); srIt != results.end(); ++srIt, ++idx) {
    indri::api::ScoredExtentResult sr = *srIt;
    dumpStream << teamId << " " << _runId << " " << trecIds[idx] << " " << queryId << " " << "1000" << " " << "2" << " " << "1" << " " << dirs[idx] << " " << "NULL" << " " << "-1" << "0-0" <<  "  " << sr.score <<"\n";
  }
  dumpStream.close();
}

std::vector<indri::api::DocumentVector*> QueryThread::getDocumentVector() {
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<indri::api::ScoredExtentResult>::iterator srIt = _results.begin(); srIt != _results.end(); ++srIt)
    docIds.push_back((*srIt).document);
  return _environment.documentVectors(docIds);
}

std::vector<std::string> QueryThread::getMetadata(std::string metaKey) {
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<indri::api::ScoredExtentResult>::iterator srIt = _results.begin(); srIt != _results.end(); ++srIt)
    docIds.push_back((*srIt).document);
  return _environment.documentMetadata(docIds, metaKey);
}


float QueryThread::getScore(int index) {
  return (_results[index]).score;
}
lemur::api::DOCID_T QueryThread::getDocId(int idx) {
  return (_results[idx]).document;
}

std::vector<lemur::api::DOCID_T> QueryThread::getDocIds() {
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<indri::api::ScoredExtentResult>::iterator srIt = _results.begin(); srIt != _results.end(); ++srIt)
    docIds.push_back((*srIt).document);
  return docIds;
}

indri::api::DocumentVector* QueryThread::getDocumentVector(indri::api::ScoredExtentResult& sr) {
  std::vector<lemur::api::DOCID_T> docIds;
  docIds.push_back(sr.document);
  std::vector<indri::api::DocumentVector*> dvs = _environment.documentVectors(docIds);
  if (dvs.size() <= 0)
    return 0;
  return dvs[0];
}

unsigned long QueryThread::documentCount() {
  return _environment.documentCount();
}

unsigned long QueryThread::documentCount(std::string& term) {
  return _environment.documentCount(term);
}

unsigned long QueryThread::termCount(std::string& term, bool stem) {
  return stem? _environment.stemCount(term) : _environment.termCount(term);
}

unsigned long QueryThread::termCount() {
  return _environment.termCount();
}

void QueryThread::unsetExpander() {
  if(_expander != 0)
      delete _expander;      
}

void QueryThread::deinitialize() {
  if(_expander != 0)
    delete _expander;
  _results.clear();
  _environment.close();
}

QueryThread::~QueryThread() {
  if(_expander != 0)
    delete _expander;
  _environment.close();
}
