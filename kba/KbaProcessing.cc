#include "indri/ThriftDocumentExtractor.hpp"
#include "indri/FileTreeIterator.hpp"

int main(int argc, char *argv[]){
  
  if (argc < 2) {
    cout << "Usage : specify a file or base dir to process";
    return -1;
  }
  
  std::string basePath(argv[1]);
  indri::file::FileTreeIterator files(basePath);
  
  for(; files != indri::file::FileTreeIterator::end() ;files++) {
    indri::parse::ThriftDocumentExtractor docExtractor;  
    std::string fileName(*files);
    cout << "\n processing " << fileName; 
    docExtractor.open(fileName);
    StreamItem *streamItem;
    while((streamItem = docExtractor.nextStreamItem()) != NULL) {
      docExtractor.iterateOverRelations(*streamItem);
    }
  }
  return 0;
}
