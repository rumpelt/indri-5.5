#include "indri/ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/tokenizer.hpp"
#include "tokenize.hpp"

#include <fstream>

namespace cmndOp = boost::program_options;


void iterateOnStream(std::string& fileName, cmndOp::variables_map& cmndMap, std::string& taggerId) {
  indri::parse::ThriftDocumentExtractor docExtractor;   

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

int main(int argc, char *argv[]){
  std::string taggerId;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help","the help message")
    ("file",cmndOp::value<std::string>(),"the file or base dir to process")
    ("anchor"," print anchor text")
    ("title"," print title text")
    ("sentence"," print sentences ")
    ("taggerId",cmndOp::value<std::string>(&taggerId)->default_value("lingpipe")," print anchor text");

  cmndOp::variables_map cmndMap;
  cmndOp::store(cmndOp::parse_command_line(argc, argv,cmndDesc), cmndMap);
  cmndOp::notify(cmndMap);  
  
  
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
