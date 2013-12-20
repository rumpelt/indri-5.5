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

  query_t( int _index, std::string _id, const std::string& _text, const std::string &queryType,  std::vector<std::string> workSet,   std::vector<std::string> FBDocs) :
    index( _index ),
    id( _id ),
    text( _text ), qType(queryType), workingSet(workSet), relFBDocs(FBDocs)
  {
  }

  query_t( int _index, std::string _id, const std::string& _text ) :
    index( _index ),
    id(_id),
    text( _text )
  {
  }

  std::string id;
  int index;
  std::string text;
  std::string expandedText; // Contains the expanded query
  std::string qType;
  // working set to restrict retrieval
  std::vector<std::string> workingSet;
  // Rel fb docs
  std::vector<std::string> relFBDocs;
  std::vector<indri::api::DocumentVector*> docs;
  ~query_t() {
    for(std::vector<indri::api::DocumentVector*>::iterator docIt = docs.begin(); docIt != docs.end(); ++docIt) 
      delete *docIt;
    docs.clear();
  };
};


class QueryThread {
private:
  //std::queue<query_t*>  _queries;
  /**
   * The actual query envrionment
   */
  indri::api::QueryEnvironment _environment; // the query environment
  /**
   * The parameters uploaded from parameter files
   */
  indri::api::Parameters _parameters;
  /**
   * Are we going to expand the query using pseudo relevance feedback. 
   * This is specified as part of the parameters file.
   */
  indri::query::QueryExpander* _expander;
  /**
   * The current results set as per the current run. 
   */
  std::vector<indri::api::ScoredExtentResult> _results;

  /**
   * List of index directories
   */
  std::vector<std::string> _indexDirs;
  /**
   * number results requested.
   */
  int _requested;

  /**
   *  Intially requested set of results for pseudo relevance feedback;
   */
  int _initialRequested;

  std::string _runId;
public:
  QueryThread(indri::api::Parameters params, std::vector<std::string> indexDirs);
  UINT64 initialize();
  /**
   * Must call to free up the resources
   */
  void deinitialize();
  /**
   * run a text query, QueryType is always "indri"
   */
  void _runQuery(std::string& queryText, std::string& queryType);
  void unsetExpander();
  void dumpKbaResult(std::vector<indri::api::ScoredExtentResult>&, std::string query, std::string dumfile); 
  /**
   * Does nothing
   */
  ~QueryThread();
};
#endif
