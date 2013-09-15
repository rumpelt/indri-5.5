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
    snprintf(options,bufsize,optionFmt.c_str(), "true", _hashType.c_str(),dirToStore.c_str());
  else
    snprintf(options,bufsize,optionFmt.c_str(), "false", _hashType.c_str(),dirToStore.c_str());
  
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
						       


RDFParser::RDFParser(std::string parserName = "ntriples", std::string hashType="bdb") {
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
  
  if(argc < 3) {
    std::cout << "usage directory To Store and director to parse needs to be specified";
    return -1;
  }

  RDFParser rdfparser;
  bool newRepo = true;;
  std::string dirToStore(argv[2]);
  std::string repoName = "test";
  
  struct dirent *dirStruct;
  DIR *directory =  opendir(argv[1]);

  rdfparser.initRDFParser(repoName, dirToStore, newRepo);

  if(directory != NULL) {
    while((dirStruct = readdir(directory)) != NULL) {
      std::string fileName(dirStruct->d_name);
      if(fileName.compare(".") ==0  || fileName.compare("..") == 0)
        continue;   
      std::string fullName = "file://"+std::string(argv[1])+"/"+fileName;
      rdfparser.parse(fullName);
      std::cout << fileName +"\n";   
    } 
  }
  else {
 
    FILE* infile = fopen(argv[1], "r");
    if (infile != NULL)  {
      fclose(infile);
      std::string infile(argv[1]);
      infile = "file://" + infile;
      rdfparser.parse(infile); 
    }
    else {
      std::cout << "cannot open input file ";
    }
  } 
}
