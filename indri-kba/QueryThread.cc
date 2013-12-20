#include "QueryThread.hpp"

QueryThread::QueryThread(indri::api::Parameters params, std::vector<std::string> dirs) : 
  _parameters(params), _indexDirs(dirs){}

UINT64 QueryThread::initialize() {

  _environment.setSingleBackgroundModel( QueryThread::_parameters.get("singleBackgroundModel", false));
  
  std::vector<std::string> stopwords;
  if( copy_parameters_to_string_vector( stopwords, _parameters, "stopper.word" ))
    _environment.setStopwords(stopwords);
  
  std::vector<std::string> smoothingRules;
  if( copy_parameters_to_string_vector( smoothingRules, _parameters, "rule" ) )
    _environment.setScoringRules( smoothingRules );

  for(std::vector<std::string>::iterator dirIt = _indexDirs.begin(); dirIt != _indexDirs.end(); ++dirIt) {
    _environment.addIndex(*dirIt);
  }
  
  if( _parameters.exists("maxWildcardTerms") )
    _environment.setMaxWildcardTerms(_parameters.get("maxWildcardTerms", 100));

  _requested = _parameters.get( "count", 1000 );
  _initialRequested = _parameters.get( "fbDocs", _requested );
  _runID = _parameters.get( "runID", "kba" ); 
  
  if (_parameters.exists("baseline")) {
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
void QueryThread::deinitialize() {
  delete _expander;
  _environment.close();
}
