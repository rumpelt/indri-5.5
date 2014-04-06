#ifndef THRIFTREADER_HPP
#define THRIFTREADER_HPP
#include <string>
#include "ThreadQueue.hpp"
#include "ParsedStream.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "StreamUtils.hpp"
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"

class ThriftReader {
private:
public:
  void readFile(std::string fName, std::string directory, Threadsafe_Queue<kba::stream::ParsedStream> queue);
};
#endif
