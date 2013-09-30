/**
 * Ashwani : Redland library for parsing data stored in rdf format.
 */
#ifndef RDFPARSER_HPP
#define RDFPARSER_HPP

#include "indri/DirectoryIterator.hpp"

#include <stdio.h>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"

#ifdef _cplusplus
extern "C" { 
#endif
  #include <redland.h>
#ifdef _cplusplus
}
#endif

class RDFParser {

private:
  librdf_world* _world;
  librdf_storage* _storage;
  librdf_parser* _parser;
  librdf_model* _model;
  std::string _parserName; // By default we set to "ntriples", other valid values "rdfxml", "turtle"
  std::string _hashType; // The format of the datastore, by defailt it is berkley data base , i.e. "bdb"
public:    
  void  streamModel(FILE* fp);
  librdf_model* getModel();
  librdf_world* getWorld();
  librdf_parser* getParser();

  /**
  * uriInput: rdf file to parse
  * storageName : name of database store created.
  * options : option specifying the store creation. Refer to redland documentation. We use following  string : "new='no',hash-type='bdb'"
  */
  void initRDFParser(std::string& storageName, std::string& dirToStore, bool newRepository = false);

  /**
  * initializes the _parser field of this class. If it is already assgined the frees
  * the parser and reinits. Bad design.
  */
  librdf_parser* initParser(std::string parserName);
  std::string convertNodeToString(librdf_node *node);

  /**
  * must call the librdf_free_stream from caller.
  */
  librdf_stream* returnStreamFromRDFFile(librdf_parser* parser, std::string fleName, librdf_uri* baseUri);

  /**
   * must call the stream 
   * must free the stream.
   */
  librdf_stream* returnStreamFromRDFFile(librdf_parser *parser,FILE *fp, librdf_uri* baseUri);
  void iterateOnRDFStream(librdf_stream* stream);

  void iterateOnQueryResults(librdf_query_results* results);
  librdf_query_results* executeQuery( const std::string& query_string, const std::string& queryLanguage);      
  void parse(std::string& uriInput);

  RDFParser(std::string parserName="ntriples", std::string hashType="bdb");
  RDFParser(std::string dirToStore, std::string repoName, std::string parserName, std::string hashType, bool newRepository);

  
  ~RDFParser();
};

#endif
