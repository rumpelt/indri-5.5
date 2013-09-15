/**
 * Ashwani : Redland library for parsing data stored in rdf format.
 */
#ifndef INDRI_RDFPARSER_HPP
#define INDRI_RDFPARSER_HPP
#include "indri/DirectoryIterator.hpp"

#include <stdio.h>
#include <iostream>
#ifdef _cplusplus
extern "C" { 
#endif
  #include <redland.h>
#ifdef _cplusplus
}
#endif

namespace indri
{
  namespace parser
  {
    class RDFParser {

    private:
    
      librdf_world* _world;
      librdf_storage* _storage;
      librdf_parser* _parser;
      librdf_model* _model;
      std::string _parserName; // By default we set to "ntriples", other valid values "rdfxml", "turtle"
      std::string _hashType; // The format of the datastore, by defailt it is berkley data base , i.e. "bdb"
    public:    
      /**
       * uriInput: rdf file to parse
       * storageName : name of database store created.
       * options : option specifying the store creation. Refer to redland documentation. We use following  string : "new='no',hash-type='bdb'"
       */
      void initRDFParser(std::string& storageName, std::string& dirToStore, bool newRepository);
      
      void parse(std::string& uriInput);
      RDFParser(std::string parserName, std::string hashType);          
      ~RDFParser();
    };  
  }
}
#endif
