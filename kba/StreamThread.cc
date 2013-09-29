#include "StreamThread.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "StreamUtils.hpp"
#include "DumpKbaResult.hpp"

void kba::StreamThread::operator()() {
  // do nothing
}

kba::StreamThread::StreamThread(std::string path, std::string dumpFileName) {
  kba::StreamThread::_fileName = path;
  _dumpFileName = dumpFileName;
  _scorer = 0;
}

/**
 * can return empty string
 */

std::string kba::StreamThread::extractDirectoryName(std::string absoluteName) {
  size_t lastSlash = absoluteName.find_last_of('/');
  std::string dirName;
  if(lastSlash != std::string::npos) {
    size_t scndLastSlash = absoluteName.find_last_of('/', lastSlash - 1);
    if (scndLastSlash != std::string::npos) {
      dirName = absoluteName.substr(scndLastSlash + 1 , lastSlash - scndLastSlash -1);
    }
  }
  return dirName;
}

void kba::StreamThread::parseFile() {
  kba::thrift::ThriftDocumentExtractor tdextractor(StreamThread::_fileName);
  streamcorpus::StreamItem* streamItem = 0;
  while((streamItem = tdextractor.nextStreamItem()) != 0) {
    std::string title = streamcorpus::utils::getTitle(*streamItem);
    std::string id = streamItem->stream_id;
    std::vector<Entity*> entities = kba::StreamThread::_scorer->getEntityList();
    for(std::vector<Entity*>::iterator entIt = entities.begin(); entIt != entities.end(); entIt++) {
      Entity* entity = *entIt;
      int score = kba::StreamThread::_scorer->score(streamItem, entity, 600);
      kba::dump::addToResultRows(kba::StreamThread::_dumpFileName, streamItem->stream_id, entity->wikiURL, score, kba::StreamThread::extractDirectoryName(kba::StreamThread::_fileName));
    }
  }
}

kba::StreamThread::StreamThread() {
  _scorer = 0;
}


