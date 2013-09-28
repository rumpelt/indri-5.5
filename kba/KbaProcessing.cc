#include "ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/tokenizer.hpp"
#include <boost/shared_ptr.hpp>
#include "Tokenize.hpp"
#include "RDFParser.hpp"
#include "RDFQuery.hpp"
#include <fstream>
#include <vector>
#include <iostream>
namespace cmndOp = boost::program_options;


void iterateOnStream(std::string& fileName, cmndOp::variables_map& cmndMap, std::string& taggerId) {
  kba::thrift::ThriftDocumentExtractor docExtractor;   

  docExtractor.open(fileName);
  StreamItem *streamItem;
  while((streamItem = docExtractor.nextStreamItem()) != NULL) {
    if(cmndMap.count("anchor"))
        docExtractor.getAnchor(*streamItem);
    if(cmndMap.count("title"))
      docExtractor.getTitle(*streamItem); 
    if(cmndMap.count("sentence")) {
      docExtractor.iterateOverSentence(*streamItem, taggerId); 
    }
  }
}

void scoreBaseLine(std::string fileName) {
}
int main(int argc, char *argv[]){
  std::string taggerId;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help","the help message")
    ("file",cmndOp::value<std::string>(),"the file or base dir to process")
    ("anchor"," print anchor text")
    ("title"," print title text")
    ("sentence"," print sentences ")
    ("repo",cmndOp::value<std::string>(), "Repository path of the rdfs store")
    ("repo-name", cmndOp::value<std::string>(), "The name of repository in the repository path") 
    ("print-model", "print the model")
    ("equery",cmndOp::value<std::string>(), "Look up an entity")
    ("taggerId",cmndOp::value<std::string>(&taggerId)->default_value("lingpipe")," print anchor text");

  cmndOp::variables_map cmndMap;
  cmndOp::store(cmndOp::parse_command_line(argc, argv,cmndDesc), cmndMap);
  cmndOp::notify(cmndMap);  
 
  RDFParser* rdfparser = NULL; 
  if(cmndMap.count("repo") && cmndMap.count("repo-name")) {
    rdfparser = new RDFParser();   
    std::string repoName = cmndMap["repo-name"].as<std::string>();
    std::string repo = cmndMap["repo"].as<std::string>();
    rdfparser->initRDFParser(repoName, repo);

    if(cmndMap.count("print-model")) {
      rdfparser->streamModel(stdout);   
    }

    RDFQuery* rdfquery = new RDFQuery(rdfparser->getModel(), rdfparser->getWorld());
    if(cmndMap.count("equery")) {
      std::string equery = cmndMap["equery"].as<std::string>();
 
      std::cout << "Executing : " << equery <<"\n";
      std::vector< boost::shared_ptr<unsigned char> >  nodes = rdfquery->getSourceNodes((const unsigned char*)"http://xmlns.com/foaf/0.1/homepage", (const unsigned char*)equery.c_str(), false);
      for(std::vector<boost::shared_ptr<unsigned char> >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); nodeIt++) {
	boost::shared_ptr<unsigned char> nodeValue = *nodeIt;
	std::cout << "\n found nodes are "<< nodeValue.get(); 
      }
    }
  }

  if (!cmndMap.count("file")) {
    std::cout <<  "no input file specified use --file \n";
    return -1;
  }
 
  std::string basePath = cmndMap["file"].as<std::string>();
  bool isDirectory = indri::file::Path::isDirectory(basePath );   
  indri::file::FileTreeIterator files(basePath);
  if(!isDirectory)
    iterateOnStream(basePath, cmndMap, taggerId);
  else {
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      
      std::string fileName(*files);
      iterateOnStream(fileName, cmndMap, taggerId);
    }
  }
  return 0;
}
