#include "StreamThread.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "StreamUtils.hpp"

void kba::StreamThread::operator()() {
  // do nothing
}

kba::StreamThread::StreamThread(std::string path) {
  kba::StreamThread::_fileName = path;
  _scorer = 0;
}

void kba::StreamThread::parseFile() {
  kba::thrift::ThriftDocumentExtractor tdextractor(StreamThread::_fileName);
  streamcorpus::StreamItem* streamItem = 0;
  while((streamItem = tdextractor.nextStreamItem()) != 0) {
    std::string title = streamcorpus::utils::getTitle(*streamItem);
    std::string id = streamItem->stream_id;
    int score = kba::StreamThread::_scorer->score(streamItem, 600);
  }
}

kba::StreamThread::StreamThread() {
  _scorer = 0;
}


