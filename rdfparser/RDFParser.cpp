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
  
using namespace indri::parser; 
namespace cmdArg = boost::program_options;

/**
 * The uriInput should be in the URI naming convention.
 */
void indri::parser::RDFParser::parse(std::string& uriInput) {
  librdf_uri *uriToParse = librdf_new_uri(RDFParser::_world, (const unsigned char*)uriInput.data());
  if(!uriToParse) {
    fprintf(stderr, "%s: Failed to create Uri \n", uriInput.c_str());
    return;
  }
  if(librdf_parser_parse_into_model(RDFParser::_parser, uriToParse, NULL, RDFParser::_model)) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", uriInput.c_str());
  }
  librdf_free_uri(uriToParse);
}

librdf_parser* indri::parser::RDFParser::initParser(std::string parserName) {

  if(RDFParser::_parser != NULL) {
    librdf_free_parser(RDFParser::_parser);
  }
  RDFParser::_parser=librdf_new_parser(RDFParser::_world, parserName.c_str(), NULL, NULL); //  
  if(!RDFParser::_parser) {
    fprintf(stderr, "%s: Failed to create new parser 'rdfxml'\n", parserName.c_str());
   }
  return RDFParser::_parser;
}

void indri::parser::RDFParser::initRDFParser(std::string& storageName, std::string& dirToStore, bool newRepository) {
   
  int bufsize = 4096;
  char options[bufsize]; // buff to manufacture the options for creating the repository.
  std::string optionFmt = "new='%s',hash-type='%s',dir='%s'";
  int preLength = optionFmt.size()-6; // -6  for the three %s
  int allowedPathLength = bufsize - preLength;
  if (dirToStore.size() > allowedPathLength+1) {
    std::cout << "\nPath name of the rdf file is too long";
  }
  if(newRepository)
    snprintf(options,bufsize,optionFmt.c_str(), "yes", _hashType.c_str(),dirToStore.c_str());
  else {
    optionFmt = "hash-type='%s',dir='%s'";
    snprintf(options,bufsize,optionFmt.c_str(), _hashType.c_str(),dirToStore.c_str());
  }
  
  RDFParser::_storage = librdf_new_storage(RDFParser::_world, "hashes", storageName.c_str(), options);
  if(!RDFParser::_storage) {
    fprintf(stderr,"%s: Failed to create new storage for rdf data\n", dirToStore.c_str());
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

    librdf_node* predicate = librdf_statement_get_predicate(statement);
    

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
  _storage = NULL; 
  _parser = NULL;
  _model = NULL;
  _hashType =  hashType;
}
 
RDFParser::~RDFParser() {
  if (!_world)
    librdf_free_world(RDFParser::_world);
  if (!_storage)
    librdf_free_storage(RDFParser::_storage);
  if(!_parser)
    librdf_free_parser(RDFParser::_parser);
  if(! _model)
    librdf_free_model(RDFParser::_model);
}

static void iterationTest(RDFParser rdfparser, std::string inFileUri) {
  librdf_parser* parser = rdfparser.initParser("ntriples");
  librdf_stream* stream = rdfparser.returnStreamFromRDFFile(parser, inFileUri, NULL);
  rdfparser.iterateOnRDFStream(stream);
}

int main(int argc, char* argv[]) {
  bool newRepository = false;
  cmdArg::options_description cmdDesc("Allowed command line options");

  cmdDesc.add_options()
    ("help","the help message")
    ("parse",cmdArg::value<std::string>(),"the file or base dir to parse")
    ("query",cmdArg::value<std::string>(),"query file or string to process")
    ("repo",cmdArg::value<std::string>(),"Repository to query/store data")
    ("repo-name",cmdArg::value<std::string>(),"Name of the repository..the files in repo will be created with this name")
    ("new-repo",cmdArg::value<bool>(&newRepository)->default_value(false),"true/false option to create/use-existing repository");  
  
  cmdArg::variables_map cmdMap;
  cmdArg::store(cmdArg::parse_command_line(argc, argv,cmdDesc), cmdMap);
  cmdArg::notify(cmdMap);  
   
  RDFParser rdfparser{};
  bool newRepo = false;
  std::string dirToStore;
  std::string parsePath;
  std::string repoName;

  if(cmdMap.count("parse")) {
    parsePath = cmdMap["parse"].as<std::string>();   // You need to remove the last character / from path
    
  }
  
  if(cmdMap.count("repo") && cmdMap.count("repo-name")) {
    dirToStore = cmdMap["repo"].as<std::string>();
    repoName = cmdMap["repo-name"].as<std::string>();;
    rdfparser.initRDFParser(repoName, dirToStore, newRepo);
  }
  else {
    std::cout << "repository path or repository name not specified, use --repo-name and --repo\n";
  }

  
  struct dirent *dirStruct;
  DIR *directory =  opendir(parsePath.c_str());
  
  
   
  if(directory != NULL) {
    while((dirStruct = readdir(directory)) != NULL) {
      std::string fileName(dirStruct->d_name);
      if(fileName.compare(".") ==0  || fileName.compare("..") == 0 || fileName.rfind(".nt") == std::string::npos)
        continue;   
      std::string fullName = "file://"+parsePath+"/"+fileName;
      
      rdfparser.parse(fullName);
      std::cout << "parsed file : "+fullName + "\n" ; 
      
    } 
  }
  else {
    FILE* infilePtr = fopen(parsePath.c_str(), "r");
    if (infilePtr != NULL)  {
      fclose(infilePtr);
      std::string inFile;  
      if (parsePath.rfind(".nt") != std::string::npos) {
        inFile = "file://" + parsePath;
	//        rdfparser.parse(inFile); 
        iterationTest(rdfparser, parsePath);
	std::cout << "parsed file :"+ inFile + "\n";
      }
    }
    else {
      std::cout << "cannot open input file : " << parsePath.c_str();
    }
  } 
}
