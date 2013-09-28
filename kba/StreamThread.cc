#include "StreamThread.hpp"

void kba::StreamThread::operator()() {
  // do nothing
}

kba::StreamThread::StreamThread(std::string path) {
  kba::StreamThread::_file = fopen(path.c_str(), "r");
}

kba::StreamThread::StreamThread() {
  kba::StreamThread::_file = 0;
}

kba::StreamThread::~StreamThread() {
  if(kba::StreamThread::_file != NULL) 
    fclose(kba::StreamThread::_file);
}
