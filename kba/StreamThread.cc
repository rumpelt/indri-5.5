#include "StreamThread.hpp"
#include "ThriftDocumentExtractor.hpp"
#include "StreamUtils.hpp"
#include "DumpKbaResult.hpp"
#include <iostream>
#include <map>

void kba::StreamThread::operator()() {
  //std::cout << "processing Stream thread \n";
  kba::StreamThread::parseFile();
}

kba::StreamThread::StreamThread(std::string path, std::fstream* dumpStream, kba::scorer::Scorer* scorer, boost::mutex *lockMutex) {
  kba::StreamThread::_fileName = path;
  _dumpStream = dumpStream;
  _scorer = scorer;
  _lockMutex = lockMutex;
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

void kba::StreamThread::updateScore(std::vector<kba::dump::ResultRow>& rows, std::string& streamId, int& score) {

  for (int index=0; index < rows.size(); index++) {
    kba::dump::ResultRow row = rows[index];
    if(!row.streamId.compare(streamId)) {
      row.score = score;
    }
  }

}

void kba::StreamThread::parseFile() {

  kba::thrift::ThriftDocumentExtractor* tdextractor= new kba::thrift::ThriftDocumentExtractor();
  tdextractor->open(StreamThread::_fileName);
  streamcorpus::StreamItem* streamItem = 0;
  std::vector<kba::dump::ResultRow> rows;

  while((streamItem = tdextractor->nextStreamItem()) != 0) {
    std::string title = streamcorpus::utils::getTitle(*streamItem);
    std::string id = streamItem->stream_id;
    std::map<std::string, int> streamScores; //required because a stream contain duplicates id..This map is defined here to avoid object creation at the start of new iteration of entity;
    int prevStreamScore;
    //std::cout << "id :: "<< id << "\n";
    std::vector<kba::entity::Entity*> entities = kba::StreamThread::_scorer->getEntityList();
    for(std::vector<kba::entity::Entity*>::iterator entIt = entities.begin(); entIt != entities.end(); entIt++) {
      
      
      prevStreamScore = -1;
      if(streamScores.find(id) != streamScores.end())
        prevStreamScore = streamScores[id];
 
      kba::entity::Entity* entity = *entIt;
      //std::cout << "scoring : " << entity->wikiURL << "\n";
      int score = kba::StreamThread::_scorer->score(streamItem, entity, 600);
      if(prevStreamScore > -1 && score > prevStreamScore) {
	kba::StreamThread::updateScore(rows, id, score);  
        streamScores.erase(id);
        streamScores[id]= score;
      }
      else if (prevStreamScore < 0 && score >= 300) {
        std::string dateHour = kba::StreamThread::extractDirectoryName(StreamThread::_fileName);
        kba::dump::ResultRow row = kba::dump::makeCCRResultRow(id, entity->wikiURL, score, dateHour);
        streamScores[id] = score;
        rows.push_back(row);  
      }
      if(rows.size() > 1000 && _lockMutex != 0) {
        boost::lock_guard<boost::mutex> lockg(*_lockMutex); 
        kba::dump::flushToDumpFile(rows, _dumpStream);
        rows.clear();
      }
      streamScores.clear(); // clear the map to be used for next iteartion of Entity 
    }
    
  }

  if(rows.size() > 0 && _lockMutex != 0) {
    boost::lock_guard<boost::mutex> lockg(*_lockMutex); 
    kba::dump::flushToDumpFile(rows, _dumpStream);
    rows.clear();
  } 

  delete tdextractor;
  //  std::cout << "Acquiring lock :"<< StreamThread::_fileName << "\n";
  
}

kba::StreamThread::StreamThread() {
  _scorer = 0;
}


