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

#include "Distribution.hpp"
#include "Passage.hpp"

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

  query_t( int _index, std::string _id, const std::string _text, const std::string queryType,  std::vector<std::string> workSet,   std::vector<std::string> FBDocs) :
    index( _index ),
    id( _id ),
    text( _text ), qType(queryType), workingSet(workSet), relFBDocs(FBDocs)
  {
  }

  query_t( int _index, std::string _id, const std::string _text , std::string queryType) :
    index( _index ),
    id(_id),
    text( _text ),
    qType(queryType)
  {
  }

  std::string id;
  int index;
  std::string text;// the original query
  std::string description; // May contain decription or abstract
  std::vector<std::string> textVector; // Must be populated from description or text
  Distribution* distribution;
  std::string expandedText; // Contains the expanded query
  std::map<std::string, float> textWeight;
  std::vector<Passage> relevantDocs;
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

  void clearDocs() {
    for(std::vector<indri::api::DocumentVector*>::iterator docIt = docs.begin(); docIt != docs.end(); ++docIt)  {
      delete *docIt;
    }
    docs.clear();
  };

  void addDocs(std::vector<indri::api::DocumentVector*> documents) {
    for(std::vector<indri::api::DocumentVector*>::iterator docIt = documents.begin(); docIt != documents.end(); ++docIt)  {
      docs.push_back(*docIt);
    }
  }
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
  indri::api::Parameters& _parameters;
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
  UINT64 initialize();
  /**
   * Must call to free up the resources
   */
  void deinitialize();
public:
  QueryThread(indri::api::Parameters& params, std::vector<std::string> indexDirs);
  QueryThread(indri::api::Parameters& params, std::string indexDir);
  ~QueryThread();
  
  void addIndexDir(std::string dir);  
  void removeDir(std::string dir);
  /**
   * run a text query, QueryType is always "indri"
   */
  void _runQuery(std::string queryText, std::string queryType);
  void _runQuery(query_t* query, bool feedBack, int initRequestSize, int requestSize);
  void unsetExpander();

  void dumpKbaResult(std::vector<indri::api::ScoredExtentResult>&, std::string query, std::string dumfile); 
  float getScore(int index);
  std::vector<indri::api::DocumentVector*> getDocumentVector();

  lemur::api::DOCID_T getDocId(int idx);
  std::vector<lemur::api::DOCID_T> getDocIds();
  std::vector<std::string> getMetadata(std::string metaKey);
   

  indri::api::DocumentVector* getDocumentVector(indri::api::ScoredExtentResult& sr);
  unsigned long documentCount();
  unsigned long documentCount(std::string& term);
  unsigned long termCount(std::string& term,bool stem=false);
  unsigned long termCount(); // total term count in index;
};
#endif
