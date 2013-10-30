
/**
 * Ashwani: implementation for the RDF Parsing.
 */

#include "RDFParser.hpp"
#include <vector>

#ifdef _cplusplus
extern "C" {
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _cplusplus
}
#endif

#include "Tokenize.hpp"
  

librdf_model* RDFParser::getModel() {
  return RDFParser::_model;
}

librdf_world* RDFParser::getWorld() {
  return RDFParser::_world;
}

librdf_parser* RDFParser::getParser() {
  return RDFParser::_parser;
}


void RDFParser::streamModel(FILE* fp) {
  raptor_iostream* iostr;
  raptor_world *raptor_world_ptr;
  raptor_world_ptr = librdf_world_get_raptor(RDFParser::_world);
  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, fp);
  librdf_model_write(RDFParser::_model, iostr);
  raptor_free_iostream(iostr);

}

/**
 * The uriInput should be in the URI naming convention.
 */
void RDFParser::parse(std::string& uriInput) {
  librdf_uri *uriToParse = librdf_new_uri(RDFParser::_world, (const unsigned char*)uriInput.data());
  if(!uriToParse) {
    fprintf(stderr, "%s: Failed to create Uri \n", uriInput.c_str());
    return;
  }
  if(librdf_parser_parse_into_model(RDFParser::_parser, uriToParse, NULL, RDFParser::_model)) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", uriInput.c_str());
  }
  librdf_model_sync(RDFParser::_model);
  librdf_model_transaction_commit(RDFParser::_model);
  librdf_free_uri(uriToParse);
}

librdf_parser* RDFParser::initParser(std::string parserName) {

  if(RDFParser::_parser != 0) {
    librdf_free_parser(RDFParser::_parser);
  }
  RDFParser::_parser=librdf_new_parser(RDFParser::_world, parserName.c_str(), NULL, NULL); //  
  if(!RDFParser::_parser) {
    fprintf(stderr, "%s: Failed to create new parser 'rdfxml'\n", parserName.c_str());
   }
  return RDFParser::_parser;
}

void RDFParser::initRDFParser(std::string& storageName, std::string& dirToStore, bool newRepository) {
   
  int bufsize = 4096;
  char options[bufsize]; // buff to manufacture the options for creating the repository.
  std::string optionFmt = "index-predicates='no',contexts='no',new='%s',hash-type='%s',dir='%s'";
  int preLength = optionFmt.size()-6; // -6  for the three %s
  int allowedPathLength = bufsize - preLength;
  if ((int)dirToStore.size() > allowedPathLength+1) {
    std::cout << "\nPath name of the rdf file is too long";
  }
  if(newRepository)
    snprintf(options,bufsize,optionFmt.c_str(), "yes", _hashType.c_str(),dirToStore.c_str());
  else {
    optionFmt = "hash-type='%s',new='no',dir='%s'";
    snprintf(options,bufsize,optionFmt.c_str(), _hashType.c_str(),dirToStore.c_str());
  }
  
  RDFParser::_storage = librdf_new_storage(RDFParser::_world, "hashes", storageName.c_str(), options);
  if(!RDFParser::_storage) {
    fprintf(stderr,"%s: Failed to create new storage for rdf data and storage name %s\n", dirToStore.c_str(), storageName.c_str());
    return;
  }


  RDFParser::_model = librdf_new_model(RDFParser::_world,RDFParser::_storage, NULL);
  if(!RDFParser::_model) {
    fprintf(stderr, "%s: Failed to create model\n", dirToStore.c_str());
    return;
  }
  
  RDFParser::_parser=librdf_new_parser(RDFParser::_world, RDFParser::_parserName.c_str(), NULL, NULL); //  
  if(!RDFParser::_parser) {
    fprintf(stderr, "%s: Failed to create new parser 'rdfxml'\n", dirToStore.c_str());
   }
  return;
}

/**
Read a rdf file and return streeam to it.
Do not forget to close the stream from the function calling this.
Opens the file internally and close it at end.
fileName should be full path name.
 */
librdf_stream* RDFParser::returnStreamFromRDFFile(librdf_parser* parser,std::string fileName,  librdf_uri* baseUri) {
  std::cout << "parsing file "<< fileName.c_str();
  FILE* fp =fopen(fileName.c_str(),"r");
  return librdf_parser_parse_file_handle_as_stream(parser,fp,1,baseUri); 
}


std::string RDFParser::convertNodeToString(librdf_node* node) {
  std::string nodeValue;

  if(librdf_node_is_blank(node))
    return nodeValue;
  else if(librdf_node_is_resource(node)) {
    librdf_uri *uri = librdf_node_get_uri(node);
    unsigned char* uriString  = librdf_uri_to_string(uri);
    if(uriString != NULL) {
      nodeValue = (const char*) uriString;
      free(uriString);
    }
    return nodeValue; 
  }
  else if(librdf_node_is_literal(node))
    nodeValue = (const char*)librdf_node_get_literal_value(node);

  return nodeValue;
}

void RDFParser::iterateOnRDFStream(librdf_stream *stream) {

  while(!librdf_stream_end(stream)) {
    librdf_statement* statement = librdf_stream_get_object(stream); // this call return a shared pointer and so we need not worry about it as we use this only in this scope
    
    librdf_node* subject = librdf_statement_get_subject(statement);   
    std::string nodeText = RDFParser::convertNodeToString(subject);

    std::cout << "subject : " << nodeText;

    //    librdf_node* predicate = librdf_statement_get_predicate(statement);
    

    librdf_node* object = librdf_statement_get_object(statement);
    nodeText = RDFParser::convertNodeToString(object);
    std::cout << " object : " << nodeText << "\n";
    librdf_stream_next(stream);
  }
}
/**
Read a rdf file and return streeam to it.
Do not forget to close the stream from the function calling this.
Opens the file internally and close it at end.
 */
librdf_stream* RDFParser::returnStreamFromRDFFile(librdf_parser* parser,FILE* fp, librdf_uri* baseUri) {
  return librdf_parser_parse_file_handle_as_stream(parser,fp,1,baseUri); 
}
						       
void RDFParser::iterateOnQueryResults(librdf_query_results* results) {
  while(!librdf_query_results_finished(results)) {
    const char **names=NULL;
    librdf_node* values[10]; 
    
    if(librdf_query_results_get_bindings(results, &names, values))
      break;
    
    fputs("result: [", stdout);
    if(names) {
      int i;
      
      for(i=0; names[i]; i++) {
        fprintf(stdout, "%s=", names[i]);
        if(values[i]) {
          librdf_node_print(values[i], stdout);
          librdf_free_node(values[i]);
        } else
          fputs("NULL", stdout);
        if(names[i+1])
          fputs(", ", stdout);
      }
    }
    fputs("]\n", stdout);
    
    librdf_query_results_next(results);
  }
  
}

librdf_query_results* RDFParser::executeQuery(const std::string& query_string, const std::string& queryLanguage) {
  librdf_query* query = NULL;
  query = librdf_new_query(RDFParser::_world, queryLanguage.c_str(), NULL, (const unsigned char* )query_string.c_str(), NULL);
  if(query != NULL) {
    librdf_free_query(query);
    return librdf_model_query_execute(RDFParser::_model, query);
  }
  else
    fprintf(stderr, "Cannot create query: %s ",query_string.c_str()); 
  return NULL;
}

RDFParser::RDFParser(std::string parserName, std::string hashType) {
  RDFParser::_world = librdf_new_world();
  librdf_world_open(RDFParser::_world);
  _parserName = parserName;
  _storage = 0; 
  _parser = 0;
  _model = 0;
  _hashType =  hashType;
}



RDFParser::RDFParser(std::string dirToStore, std::string repoName, std::string parserName, std::string hashType, bool newRepository) : _world(0), _storage(0),_parser(0), _model(0){
  RDFParser::_world = librdf_new_world();
  librdf_world_open(RDFParser::_world);
  RDFParser::initRDFParser(repoName, dirToStore, newRepository);
  RDFParser::initParser(parserName);
}
 
RDFParser::~RDFParser() {
  if (!_storage)
    librdf_free_storage(RDFParser::_storage);
  if(!_parser)
    librdf_free_parser(RDFParser::_parser);
   if(! _model)
    librdf_free_model(RDFParser::_model);
  if (!_world)
    librdf_free_world(RDFParser::_world);
}

