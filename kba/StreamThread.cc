#include "StreamThread.hpp"
#include "ThriftDocumentExtractor.hpp"

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
  }
}

kba::StreamThread::StreamThread() {
  _scorer = 0;
}


