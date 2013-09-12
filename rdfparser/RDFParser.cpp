/**
 * Ashwani: implementation for the RDF Parsing.
 */

#include "indri/RDFParser.hpp"

void indri::parser::RDFParser::initRDFParser(const char* uriInput, const char* storgaeDir, cont char* storageName, const char* optoptions) {
  indri::parser::RDFParser::world = librdf_new_world();
  indri::parser::RDFParser::librdf_world_open(world);
  indri::parser::raptor_world_ptr = librdf_world_get_raptor(world); 
  indri::parser::RDFParser::uriToParse = librdf_new_uri(world,(const char*) uriInput);
  if(!uriToParse) {
    fprintf(stderr, "%s: Failed to create Uri \n", uriInput);
    return;
  } 
  indri::parse::RDFParser::storge = librdf_new_storage(world, "hashes", storageName,  
}


