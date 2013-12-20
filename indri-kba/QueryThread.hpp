#ifndef QUERYTHREAD_HPP
#define QUERYTHREAD_HPP
#include <queue>
#include <string>
#include <indri/Parameters.hpp>
#include <indri/QueryEnvironment.hpp>
#include <indri/QueryExpander.hpp>
#include "indri/TFIDFExpander.hpp"
#include "indri/RMExpander.hpp"
#include <indri/ScoredExtentResult.hpp>

static bool copy_parameters_to_string_vector( std::vector<std::string>& vec, indri::api::Parameters p, const std::string& parameterName ) {
  if( !p.exists(parameterName) )
    return false;

  indri::api::Parameters slice = p[parameterName];

  for( size_t i=0; i<slice.size(); i++ ) {
    vec.push_back( slice[i] );
  }

  return true;
}

struct query_t {
  struct greater {
    bool operator() ( query_t* one, query_t* two ) {
      return one->index > two->index;
    }
  };

  query_t( int _index, std::string _number, const std::string& _text, const std::string &queryType,  std::vector<std::string> workSet,   std::vector<std::string> FBDocs) :
    index( _index ),
    number( _number ),
    text( _text ), qType(queryType), workingSet(workSet), relFBDocs(FBDocs)
  {
  }

  query_t( int _index, std::string _number, const std::string& _text ) :
    index( _index ),
    number( _number ),
    text( _text )
  {
  }

  std::string number;
  int index;
  std::string text;
  std::string qType;
  // working set to restrict retrieval
  std::vector<std::string> workingSet;
  // Rel fb docs
  std::vector<std::string> relFBDocs;
};


class QueryThread {
private:
  std::queue<query_t*>  _queries;
  indri::api::QueryEnvironment _environment;
  indri::api::Parameters _parameters;
  indri::query::QueryExpander* _expander;
  std::vector<indri::api::ScoredExtentResult> _results;
  std::vector<std::string> _indexDirs;
  
  int _requested;
  int _initialRequested;
  std::string _runID;
public:
  QueryThread(indri::api::Parameters params, std::vector<std::string> indexDirs);
  UINT64 initialize();
  void deinitialize();
};
#endif
