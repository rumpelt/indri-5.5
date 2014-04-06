#include "ThriftReader.hpp"

void ThriftReader::readFile(std::string fileName, std::string directory, Threadsafe_Queue<kba::stream::ParsedStream> queue) {
  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  std::vector<kba::stream::ParsedStream> streams;
  tdextractor->open(fileName);
  streamcorpus::StreamItem* streamItem = 0;
  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
    std::string title = streamcorpus::utils::getTitle(*streamItem);
    std::string body  = (streamItem->body).clean_visible;
    if(body.size() > 0) {
      body = anchor + " "+ title + " " +body;
      kba::stream::ParsedStream ps(body.size());
      ps.stream_id = streamItem->stream_id;
      ps.directory = directory;
      ps.rawText = title+" " + anchor + " "+body;
    }
  }
  delete tdextractor;
  std::string test;
  //  queue.pushdata(test);
  queue.pushValues(streams);
  
}
