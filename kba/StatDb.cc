#include "StatDb.hpp"
#include <errno.h>
#include "Logging.hpp"
using namespace std;

StatDb::StatDb() : _trmFd(-1), _crpFd(-1), _tpcFd(-1), _trmTpcFd(-1), _evalFd(-1) {
  _pgSz = getpagesize();
}

StatDb::~StatDb() {
  if(_trmFd > 0) {
    Logger::LOG_MSG("StatDb.cc", "~StatDb", "closing term db");  
    close(_trmFd);
    _trmFd = -1;
  }
  if(_crpFd > 0) {
    Logger::LOG_MSG("StatDb.cc", "~StatDb", "closing corpus db");  
    close(_crpFd);
    _crpFd = -1;
  }

  if(_evalFd > 0) {
    Logger::LOG_MSG("StatDb.cc", "~StatDb", "closing eval db");  
    close(_evalFd);
    _evalFd = -1;
  }

  if(_tpcFd > 0) {
    Logger::LOG_MSG("StatDb.cc", "~StatDb", "closing topic db");  
    close(_tpcFd);
    _tpcFd = -1;
  }
  if(_trmTpcFd > 0) {
    Logger::LOG_MSG("StatDb.cc", "~StatDb", "closing term topic  db");  
    close(_trmTpcFd);
    _trmTpcFd = -1;
  }
}

void StatDb::crtTrmDb(string fName, bool rdOnly) {
  if (rdOnly)
    _trmFd = open(fName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);  
  else
    _trmFd = open(fName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if(_trmFd < 0) {
    std::cerr << "Filecreation failed for " << fName << " status " << _trmFd;
  } 
  
}
void StatDb::crtEvalDb(string fName, bool rdOnly) {
  if (rdOnly)
    _evalFd = open(fName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);  
  else
    _evalFd = open(fName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
  if(_evalFd < 0) {
    std::cerr << "File creation failed for " << fName << " status " << _evalFd;
  }
}

void   StatDb::crtCrpDb(string fName, bool rdOnly) {
  if(rdOnly)
    _crpFd = open(fName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);  
  else
    _crpFd = open(fName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
}

void StatDb::crtTpcDb(string fName, bool rdOnly) {
  if(rdOnly)
    _tpcFd = open(fName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);  
  else
    _tpcFd = open(fName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
}

void StatDb::crtTrmTpcDb(string fName, bool rdOnly) {
  if(rdOnly)
    _trmTpcFd = open(fName.c_str(), O_RDONLY, S_IRUSR | S_IWUSR);  
  else
    _trmTpcFd = open(fName.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
}

void StatDb::wrtTrmSt(TermStat* trmSt) {
  size_t totalSize = sizeof(time_t) + (trmSt->term).size() + 1 + sizeof(long) + sizeof(long);
  char* packedData =  new char[totalSize+sizeof(size_t)];
  memset(packedData, 0, totalSize + sizeof(size_t));
  memcpy(packedData, &totalSize, totalSize);
  memcpy(packedData + sizeof(size_t), &(trmSt->collectionTime), sizeof(time_t));
  memcpy(packedData + sizeof(size_t)+ sizeof(time_t), (trmSt->term).c_str(), (trmSt->term).size() + 1);
  memcpy(packedData + sizeof(size_t)+ sizeof(time_t) + (trmSt->term).size() + 1, &(trmSt->docFreq), sizeof(long));
  memcpy(packedData + sizeof(size_t)+ sizeof(time_t) + (trmSt->term).size() + 1 + sizeof(long), &(trmSt->collFreq), sizeof(long));

  if( _trmFd > 0) {
    int status =  write(_trmFd, packedData, totalSize + sizeof(size_t)); 
    if (status < 0) {
      Logger::LOG_MSG("StatDb", "wrtTrmSt", "Fatal error ,Cannot write the Term Stat\n");
    }
  }
  else {
    Logger::LOG_MSG("StatDb", "wrtTrmSt", "Fatal error , term stat db not open\n");
  }
  delete packedData;
}

void StatDb::wrtCrpSt(CorpusStat* crpSt) {
  if(_crpFd > 0) {
    size_t dataSize = sizeof(CorpusStat);
    char* packedData = new char[dataSize + sizeof(size_t)];
    memset(packedData, 0 , dataSize + sizeof(size_t));
    memcpy(packedData, &dataSize, sizeof(size_t));
    memcpy(packedData + sizeof(size_t), crpSt, sizeof(CorpusStat));
    int status = write(_crpFd, packedData, dataSize + sizeof(size_t));
    if(status < 0)
      Logger::LOG_MSG("StatDb", "wrtCrpSt", "Fatal error ,Cannot write the Corpus Stat\n");
    delete packedData;
  }
  else {
    Logger::LOG_MSG("StatDb", "wrtCrpSt", "Fatal error , Corpus stat db not open\n");
  }
}

void StatDb::wrtEvalSt(EvaluationData* evSt) {
  if(_evalFd >0) {
    size_t dtSize = sizeof(time_t) + (evSt->docId).size()+1 + (evSt->topic).size()+ 1+ sizeof(int16_t) + sizeof(int) + (evSt->directory).size() + 1;
    char* pkDt = new char[dtSize + sizeof(size_t)];
    memset(pkDt, 0, dtSize + sizeof(size_t));
    memcpy(pkDt, &dtSize, sizeof(size_t));
    memcpy(pkDt+sizeof(size_t), &(evSt->timeStamp), sizeof(time_t));
    memcpy(pkDt+sizeof(size_t)+sizeof(time_t), (evSt->docId).c_str(), (evSt->docId).size() +1);
    memcpy(pkDt+sizeof(size_t)+sizeof(time_t) + (evSt->docId).size() +1, (evSt->topic).c_str(), (evSt->topic).size() +1);
    memcpy(pkDt+sizeof(size_t)+sizeof(time_t) +(evSt->docId).size() +1 +(evSt->topic).size() +1, &(evSt->rating), sizeof(int16_t));
    memcpy(pkDt+sizeof(size_t)+sizeof(time_t)+ (evSt->docId).size() +1 +(evSt->topic).size() +1 + sizeof(int16_t), &(evSt->cleanVisibleSize), sizeof(int));
    memcpy(pkDt+sizeof(size_t)+sizeof(time_t)+ (evSt->docId).size() +1 + (evSt->topic).size() +1 + sizeof(int16_t) + sizeof(int),
	   (evSt->directory).c_str(), (evSt->directory).size() +1);
    int status = write(_evalFd, pkDt, dtSize+sizeof(size_t));
    if(status < 0)
      Logger::LOG_MSG("StatDb.cc", "wrtEvalSt", "Could Not write the evaluation data");
    delete pkDt;
  }
  else
    Logger::LOG_MSG("StatDb.cc", "wrtEvalSt", "Eval db not open");
}

std::vector<boost::shared_ptr<EvaluationData> > StatDb::getEvalData(std::string stream_id, bool seekStart, bool rstOffset) {
  off_t offset;

  if(seekStart)
    offset = lseek(_evalFd, 0, SEEK_SET);
  
  if(rstOffset)
    offset  = lseek(_evalFd, 0, SEEK_CUR); 
  
  std::vector<boost::shared_ptr<EvaluationData> > evlVec;
  time_t timeStamp = strtol(stream_id.substr(0, stream_id.find("-")).c_str(), NULL, 10);
  std::string docId = stream_id.substr(stream_id.find("-") + 1).c_str();
  
  
  bool exitRd = false;
  while(!exitRd) {
    size_t dataSize;
    int szRd = read(_evalFd, &dataSize, sizeof(size_t));
    if(szRd == sizeof(size_t)) {
      char* data = new char[dataSize];
      char*  origData = data;
      int dtRd = read(_evalFd, data, dataSize);
      if (dtRd == dataSize) {
        time_t currTm = *((time_t*)data);
        data = data + sizeof(time_t);
	std::string id = data;
        data = data + id.size() + 1;

        if( currTm  == timeStamp && !docId.compare(id)) {
	  boost::shared_ptr<EvaluationData> evlDt(new EvaluationData());
          EvaluationData* evl = evlDt.get();
          evl->timeStamp = currTm;
          evl->docId = id;
          
          evl->topic = data;
          data = data + (evl->topic).size() + 1;  
          evl->rating = *((int16_t*)data);
          data = data + sizeof(int16_t);
          evl->cleanVisibleSize = *((int*)data);
          data = data + sizeof(int);
          evl->directory = data;          
          evlVec.push_back(evlDt);
	}
        else if(currTm > timeStamp) {
          exitRd = true; 
	}
      }
      delete origData;
    }
    else {
      exitRd = true; 
    }
  }

  if(rstOffset)
    lseek(_evalFd, offset, SEEK_SET);
  return evlVec;
}

boost::shared_ptr<TermStat> StatDb::rdTrmStat(const char* term, time_t cPoint, bool seekStart, bool rstOffset) {
  using namespace boost;
  boost::shared_ptr<TermStat> trmSt;

  off_t offt = StatDb::getTrmStatOffset(cPoint, seekStart);
  std::cout << "Got offset :" << offt << "\n";
  if(offt < 0)
    return trmSt;
  off_t origOfft = offt;

  bool exitRd= false;
  while(!exitRd) { // if else statement are carefully thought abt so be careful
    size_t dataSize;
    int szRd = read(_trmFd, &dataSize, sizeof(size_t));
    if (szRd == sizeof(size_t)) {
      char* data = new char[dataSize];
      char* origPtr =data; // To save the orignal ptr for freein the resource
      size_t dtSzRd = read(_trmFd, data, dataSize);
      if (dtSzRd == dataSize) {
        time_t collTime = *((time_t*)data);
        if (collTime == cPoint) {
          data = data + sizeof(time_t);
	  std::string dtTerm = (char*) data;
          data = data + dtTerm.size() + 1;
          if (!dtTerm.compare(term)) {
	    // Now we have reached the term we were looking for.
            trmSt  = boost::shared_ptr<TermStat>(new TermStat());
	    trmSt->term = dtTerm;
            trmSt->collectionTime = collTime;
            trmSt->docFreq = *((long*)data);
            data = data + sizeof(long);
            trmSt->collFreq = *((long*)data);
            delete origPtr;
            if(rstOffset)
              lseek(_trmFd, origOfft, SEEK_SET);      
            return trmSt;
	  }
	}
        else
          exitRd = true;
      }
      else
        exitRd = true;
      delete origPtr;
      if(rstOffset)
        lseek(_trmFd, origOfft, SEEK_SET);
    }
    else {
      exitRd = true;
    }
  }
  return trmSt;
}
  

off_t StatDb::getTrmStatOffset(time_t collTm, bool seekStart) {
  bool exitRd = false;
  off_t offt;
  if(seekStart)
    offt = lseek(_trmFd, 0, SEEK_SET);
  else
    offt = lseek(_trmFd, 0, SEEK_CUR);
  if(offt < 0) {
    std::cout << "Error in lsekk " << strerror(errno) << "\n";
    return offt;
  }
  while(!exitRd) { // Be careful with if else statements here.
     size_t dataSize;
     int bread = read(_trmFd, &dataSize, sizeof(size_t));
     if(bread > 0) {
       char* data  = new char[dataSize];
       size_t dread = read(_trmFd, data, dataSize);
       if(dread == dataSize) {
         time_t currTm = *((time_t*) data);
         if(collTm == currTm) {
           delete data;
           return offt; // we had stored the offset previousl, so just return it. Need to be careful with codechanges
	 }
         else if(currTm > collTm) {
           delete data;
           std::cout << "curr Time is  greater than the time reqd " << currTm << " " << collTm<< "\n"; 
           return -1;    
	 }
         offt = lseek(_trmFd, 0, SEEK_CUR); // we are incrementing when we are going to next read
       }
       else {
         std::cout << " read error for time reading " << "\n";
         delete data;
         return -1;
       }
     }
     else
       exitRd = true;
  }
  std::cout << "Cout not find the time " << "\n";
  return -1;
} 


off_t StatDb::getCrpStatOffset(time_t collTm, bool seekStart) {

  bool exitRd = false;
  off_t offt;
  if(seekStart)
    offt =  lseek(_crpFd, 0, SEEK_SET);
  else
    offt = lseek(_crpFd, 0, SEEK_CUR);
  if(offt < 0)
    assert(false);
  while(!exitRd) {
    size_t dataSize;
    int bread = read(_crpFd, &dataSize, sizeof(size_t));
    if (bread > 0) {
      char* data  = new char[dataSize];
      size_t dread = read(_crpFd, data, dataSize);
      if(dread == dataSize) {
        time_t currTm = *((time_t*) data);
        if(collTm == currTm) {
          delete data;
          return offt;
	}
        else if (currTm > collTm) {
          delete data;
	  return -1;
	}
        offt = lseek(_crpFd, 0, SEEK_CUR);
        if(offt < 0) {
          delete data;
          return offt;
	}
      }
      else
        assert(false);
      delete data;
    }
    else {
      exitRd = true;
    }
  }
  return -1;
}

/**
 * Each invocation will sequentially read 
 */
boost::shared_ptr<CorpusStat> StatDb::rdCrpStat(time_t cPoint, bool seekStart, bool rstOffset) {
  using namespace boost;
  boost::shared_ptr<CorpusStat> crpSt;
  off_t offt = StatDb::getCrpStatOffset(cPoint, seekStart);
  off_t origOffset = offt;

  if(offt >= 0) {
    offt = lseek(_crpFd, offt, SEEK_SET);
  }
  else
    return crpSt;

  size_t dataSize;
  int bread = read(_crpFd, &dataSize, sizeof(size_t));
  
  if(bread > 0) {
    char* data  = new char[dataSize];
    char* origPtr = data;
    size_t dread = read(_crpFd, data, dataSize);
    if(dread == dataSize) {
      crpSt = boost::shared_ptr<CorpusStat>(new CorpusStat());
      CorpusStat* st = crpSt.get();
      st->collectionTime = *((time_t*)data);
      data = data + sizeof(time_t);
      st->totalDocs = *((long*)data);
      data = data  +sizeof(long);
      st->judgedSample = *((long*)data);
      data = data + sizeof(long);
      st->averageDocSize = *((int*) data);
      delete origPtr;
      if (rstOffset)
        lseek(_crpFd, origOffset, SEEK_SET);
      return crpSt;
    }
    else
      assert(false);
  }

  return crpSt;
}
