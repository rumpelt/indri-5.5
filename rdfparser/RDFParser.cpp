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
void indri::parser::RDFParser::initRDFParser(std::string& storageName, std::string& dirToStore, bool newRepository) {
  RDFParser::_world = librdf_new_world();
  librdf_world_open(RDFParser::_world);
   
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

RDFParser::RDFParser(std::string parserName = "ntriples", std::string hashType) {
  _parserName = parserName;
  _world = NULL;
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
  cmdArg::store(cmndArg::parse_command_line(argc, argv,cmdDesc), cmdMap);
  cmdArg::notify(cmdMap);  
   
  RDFParser rdfparser;
  bool newRepo = false;
  std::string dirToStore;
  if(cmdMap.count("repo"))
    dirToStore = cmdMap["repo"].as<std::string>();
  else {
    cout << "No repository specified , use --new-repo option\n";
    return 0;
  }

  std::string repoName = argv[3];
  
  struct dirent *dirStruct;
  DIR *directory =  opendir(argv[1]);

  rdfparser.initRDFParser(repoName, dirToStore, newRepo);
   
  if(directory != NULL) {
    while((dirStruct = readdir(directory)) != NULL) {
      std::string fileName(dirStruct->d_name);
      if(fileName.compare(".") ==0  || fileName.compare("..") == 0 || fileName.rfind(".nt") == std::string::npos)
        continue;   
      std::string fullName = "file://"+std::string(argv[1])+"/"+fileName;
      
      rdfparser.parse(fullName);
      std::cout << "parsed file : "+fullName + "\n" ; 
      
    } 
  }
  else {
 
    FILE* infilePtr = fopen(argv[1], "r");
    if (infilePtr != NULL)  {
      fclose(infilePtr);
      std::string inFile(argv[1]);
      if (inFile.rfind(".nt") != std::string::npos) {
        inFile = "file://" + inFile;
        rdfparser.parse(inFile); 
	std::cout << "parsed file :"+ inFile + "\n";
      }
    }
    else {
      std::cout << "cannot open input file ";
    }
  } 
}
