#include "indri/ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "boost/tokenizer.hpp"
#include <fstream>

namespace cmndOp = boost::program_options;


std::set<std::string> getStopSet(std::string stopFile) {
  std::set<std::string> stopwords;
  std::ifstream stopWordFile(stopFile.c_str()i);
  if(stopWordFile.is_open()) {
    std::string line;
    while(std::getline(stopWordFile, line)){
      stopwords.insert(line);
    }
  }
  else {
    std::cout << "cannot open the stop word file";
  }
  return stopwords;
}

std::vector<string> filterStopWords(std::vector<string> inputTokens, std::set<std::string> stopwords) 
{
  std::vector<std::string> filterWords;
  for(std::vector<string>::iterator tokIt = inputTokens.begin(); tokIt != inputTokens.end(); tokIt++) {
    std::string token = *tokIt;
    if(token.size() > 1 && stopwords.find(token) != stopwords.end()) {
      filterWords.push_back(token);
    }
  }
  return filterWords;
}

std::vector<string> getPhrases(std::string& inputSource) {
  boost::tokenizer<> tokens(inputSource);
  std::vector<string> phrases;
  std::vector<string>::iterator phraseIt;
  int index = 0;
  bool prevCaseUpper = false;
  for(boost::tokenizer<>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    bool upperCase = isupper(token[0]);
    if(!upperCase || !prevCaseUpper) {
      phrases.push_back(token); 
    }
    else {
      std::string lastElement = phrases.back();
      lastElement = lastElement + " "+token;
      phrases.pop_back();
      phrases.push_back(lastElement);
    }
    prevCaseUpper = upperCase;
  }

  return phrases;
}

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
