/**
 * Ashwani : Redland library for parsing data stored in rdf format.
 */
#ifndef INDRI_RDFPARSER_HPP
#define INDRI_RDFPARSER_HPP
#include "indri/DirectoryIterator.hpp"
#include <stdio.h>

namespace indri
{
  namespace parser
  {
    class RDFParser {

    public:
    
      static librdf_world* world;
      static librdf_storage* storage;
      static librdf_parser* parser;
      static librdf_model* model;
      static librdf_stream* stream;
      static librdf_node *subject, *predicate;
      static librdf_iterator* iterator;
      static librdf_statement *partial_statement, *statement2;
      static librdf_uri *uriToParse;
      static raptor_world *raptor_world_ptr;
      static raptor_iostream* iostr;

      static FILE* _storageDir;

      /**
       * uriInput: rdf file to parse
       * storageName : name of database store created.
       * options : option specifying the store creation. Refer to redland documentation. We use following  string : "new='no',hash-type='bdb'"
       */
      static void initRDFParser(const char* uriInput, const char* storageName, const char *dirToStore, const char *options);       
    }; 
  }
}
#endif
