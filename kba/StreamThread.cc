#include "StreamThread.hpp"
#include "StreamUtils.hpp"
#include "DumpKbaResult.hpp"
#include "LanguageModel.hpp"
#include "BM25Scorer.hpp"
#include <iostream>
#include <map>

#include "indri/FileTreeIterator.hpp"
#include "indri/Path.hpp"
using namespace kba::term;

kba::StreamThread::StreamThread(std::vector<std::string> dirsToProcess, std::fstream* dumpStream, std::vector<kba::entity::Entity*> entities, std::unordered_set<std::string> stopSet, int cutoffScore) :
  _dirsToProcess(dirsToProcess), _dumpStream(dumpStream), _entities(entities), _stopSet(stopSet) ,  _cutoffScore(cutoffScore) {
  _crpStat= 0;

}

void kba::StreamThread::setCorpusStat(CorpusStat* crpStat) {
  _crpStat = crpStat;
}

void kba::StreamThread::setTermStat(std::set<TermStat*> termSt) {
  _trmStat = termSt;
}

void kba::StreamThread::setTopicTerm(std::set<TopicTerm*> tpcTerm) {
  _tpcTrm = tpcTerm;
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

void kba::StreamThread::updateCorpusStat(CorpusStat* crpStat, long numDocs, size_t docSize) {
  crpStat->totalDocs += numDocs;
  crpStat->collectionTime = _timeStamp;
  float currentAvgDocSize = ((float)docSize)/numDocs;
  //std::cout <<  "Current doc size " <<  docSize << " num docs " << numDocs << " " << crpStat->averageDocSize <<  "\n";
  int crpAvgDoc = crpStat->averageDocSize;
  if(crpAvgDoc > 0)
    currentAvgDocSize = (currentAvgDocSize + crpAvgDoc) / 2.0;
  currentAvgDocSize += 0.5;
  _crpStat->averageDocSize = (int)(currentAvgDocSize); // rounded or floored depedinging    
  _crpStat->collectionSize += docSize;
}

 void kba::StreamThread::updateTermStat(std::set<TermStat*> termStatSet, kba::stream::ParsedStream* stream) {
//   std::cout << "TermStat update " << "\n";
  for(std::set<kba::term::TermStat*>::iterator termIt = termStatSet.begin(); termIt != termStatSet.end(); ++termIt) {
    kba::term::TermStat* trmSt  = *termIt;
    try {
      int freq = stream->tokenFreq.at(trmSt->term);
      trmSt->docFreq += 1;
      trmSt->collFreq += freq;
      trmSt->collectionTime = _timeStamp;
      //      std::cout << " Topic term " <<  trmSt->term << " " << trmSt->docFreq << " " << trmSt->collFreq << "\n";
    } catch (std::out_of_range& oor) {
    }
  }
}

void kba::StreamThread::parseFile(int cutOffScore, std::string fileName, std::string dirName, bool firstPass) {

  _tdextractor->open(fileName);
  streamcorpus::StreamItem* streamItem = 0;
  std::vector<kba::dump::ResultRow> rows;
  long numDocs = 0;
  size_t docSize = 0;
  while((streamItem = _tdextractor->nextStreamItem()) != 0) {
    kba::stream::ParsedStream* parsedStream = streamcorpus::utils::createMinimalParsedStream(streamItem,_stopSet, _termSet);
    ++numDocs;
    docSize = docSize + parsedStream->size;
    
    std::string id = streamItem->stream_id;

    for(std::vector<kba::entity::Entity*>::iterator entIt = _entities.begin(); entIt != _entities.end(); entIt++) {
        
      kba::entity::Entity* entity = *entIt;
      if(!firstPass) {
        for(std::vector<kba::scorer::Scorer*>::iterator scIt = _scorers.begin(); scIt != _scorers.end(); ++scIt) {
	  kba::scorer::Scorer* scorer = *scIt;
          int score = (int) (scorer->score(parsedStream, entity, 1000)); // first check we have implemented the parsedStreamMethod or not
	  //	  std::cout << "Score " << scorer->getModelName() << " " << score << "n";
          if (score >= cutOffScore) {
            kba::dump::ResultRow row = kba::dump::makeCCRResultRow(id, entity->wikiURL, score, dirName, scorer->getModelName());
            rows.push_back(row);  
          }
        }
      }
    }
    //    std::cout << "Updateing TermStat" << "\n";
    updateTermStat(_trmStat, parsedStream);  
    delete parsedStream; 
  }
  updateCorpusStat(_crpStat, numDocs, docSize);

  if(rows.size() > 0) {
    kba::dump::flushToDumpFile(rows, _dumpStream);
    rows.clear();
  } 

  _tdextractor->reset();
  //  std::cout << "Acquiring lock :"<< StreamThread::_fileName << "\n";
  
}

void kba::StreamThread::setTermSet(std::set<std::string> termSet) {
  _termSet = termSet;
}

void kba::StreamThread::spawnParserNScorers(bool firstPass) {
  // create the scorers here
  using namespace kba::scorer;
  std::string dayDate = _dirsToProcess.at(0).substr(0, _dirsToProcess.at(0).rfind("-"));
  _tdextractor= new kba::thrift::ThriftDocumentExtractor();
  _timeStamp = kba::time::convertDateToTime(dayDate);
  LanguageModel* lm = 0;
  BM25Scorer* bm = 0;
  if(!firstPass) {
    lm = new LanguageModel(_entities, _trmStat, _crpStat);
    _scorers.push_back(lm);
    bm = new BM25Scorer(_entities, _crpStat, _trmStat);
    _scorers.push_back(bm);
  }
  
  for(std::vector<std::string>::iterator dirIt = _dirsToProcess.begin(); dirIt != _dirsToProcess.end(); ++dirIt) {
    indri::file::FileTreeIterator files(*dirIt);
    for(; files != indri::file::FileTreeIterator::end() ;files++) {
      std::string fName = *files;
      parseFile(_cutoffScore, fName, *dirIt, firstPass);
    }
  }
  if(lm != 0)
    delete lm;
  if(bm != 0)
    delete bm;
  delete _tdextractor;
}




