#include "StreamThread.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "StreamUtils.hpp"
#include "DumpKbaResult.hpp"
#include <iostream>
#include <map>

void kba::StreamThread::operator()(int cutOffScore) {
  //std::cout << "processing Stream thread \n";
  kba::StreamThread::parseFile(cutOffScore);
}

kba::StreamThread::StreamThread(std::string path, std::fstream* dumpStream, kba::scorer::Scorer* scorer, boost::mutex *lockMutex, std::unordered_set<std::string>& stopSet) :
  _fileName(path), _dumpStream(dumpStream), _scorer(scorer), _lockMutex(lockMutex), _stopSet(stopSet), _termBase(0) {
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



void kba::StreamThread::parseFile(int cutOffScore) {

  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(StreamThread::_fileName);
  streamcorpus::StreamItem* streamItem = 0;
  std::vector<kba::dump::ResultRow> rows;

  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createParsedStream(streamItem,_stopSet);
 
    std::string title = streamcorpus::utils::getTitle(*streamItem);
  
    std::string id = streamItem->stream_id;
 
 
    //std::cout << "id :: "<< id << "\n";
    std::vector<kba::entity::Entity*> entities = kba::StreamThread::_scorer->getEntityList();
    for(std::vector<kba::entity::Entity*>::iterator entIt = entities.begin(); entIt != entities.end(); entIt++) {
      
        
      kba::entity::Entity* entity = *entIt;

      int score = (int) (kba::StreamThread::_scorer->score(parsedStream, entity, 1000)); // first check we have implemented the parsedStreamMethod or not
      
      if(score < 0) {
	score = kba::StreamThread::_scorer->score(streamItem, entity, 1000); // we can remove this altogether.
      }

      if (score >= cutOffScore) {
        std::string dateHour = kba::StreamThread::extractDirectoryName(StreamThread::_fileName);
        kba::dump::ResultRow row = kba::dump::makeCCRResultRow(id, entity->wikiURL, score, dateHour);
        rows.push_back(row);  
      }
    }

    if(StreamThread::_termBase != 0 && StreamThread::_lockMutex != 0) {
      boost::lock_guard<boost::mutex> lockTermBase(*_lockMutex); 
      kba::term::updateTermBase(parsedStream, StreamThread::_termBase);
    }

    delete parsedStream; 
  }

  if(rows.size() > 0 && _lockMutex != 0) {
    boost::lock_guard<boost::mutex> lockg(*_lockMutex); 
    kba::dump::flushToDumpFile(rows, _dumpStream);
    rows.clear();
  
  } 

  delete tdextractor;
  //  std::cout << "Acquiring lock :"<< StreamThread::_fileName << "\n";
  
}

kba::StreamThread::StreamThread() : _scorer(0), _termBase(0){
}


