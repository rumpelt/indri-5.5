#include "QueryThread.hpp"
#include <dirent.h>

QueryThread::QueryThread(indri::api::Parameters params, std::vector<std::string> dirs) : 
  _parameters(params), _expander(0), _indexDirs(dirs){}

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

void QueryThread::_runQuery(std::string& queryText ,std::string& queryType) {
  try {
    _results = _environment.runQuery( queryText, _initialRequested, queryType ); //Runs query
    if(_expander) {
      std::string expandedQuery;
      expandedQuery = _expander->expand( queryText, _results);
      _results = _environment.runQuery(expandedQuery, _requested, queryType );
    }
    std::cout << "Num results " << _results.size() << "\n";
  }
  catch(lemur::api::Exception& e) {
    _results.clear();
    LEMUR_RETHROW(e, "QueryThread::_runQuery Exception");
  }
}

void QueryThread::deinitialize() {
  if(_expander != 0)
    delete _expander;
  _environment.close();
}


QueryThread::~QueryThread() {

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



void QueryThread::unsetExpander() {
  if(_expander != 0)
      delete _expander;
   
}
