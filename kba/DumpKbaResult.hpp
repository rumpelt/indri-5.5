#ifndef DUMPKBARESULT_HPP
#define DUMPKBARESULT_HPP
#include <string>
#include <cstdio>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>

namespace kba {
  namespace dump {

    struct ResultRow {
      std::string streamId;
      std::string entityURL;
      int score;
      int maxScore;
      std::string dateHour; // the directory under which ResultRow for this stream was found
       std::string modelName;
    };

    //   extern std::vector<ResultRow> RESULTROWS;
    //    std::string _DUMPFILE;
    
    ResultRow makeCCRResultRow(std::string& streamId, std::string& entityURL, int& score, std::string& dateHour, std::string& modelName, int& maxScore);
          
    void flushToDumpFile(std::vector<ResultRow>& rows, std::fstream* dumpStream);
    void writeHeader(std::string dumpFile);
    std::string&  rowHeader(); 
    std::vector<ResultRow>  resultRows(); 
  }
}

inline kba::dump::ResultRow kba::dump::makeCCRResultRow(std::string& streamId, std::string& entityURL, int& score, std::string& dateHour, std::string& modelName, int& maxScore) {
  ResultRow resultrow;
  resultrow.streamId = streamId;
  resultrow.entityURL = entityURL;
  resultrow.score = score;
  resultrow.maxScore = maxScore;
  resultrow.dateHour = dateHour;
  resultrow.modelName = modelName;
  return resultrow;
}

/**
 * access must be synchronized
 */
inline void kba::dump::flushToDumpFile(std::vector<ResultRow>& rows, std::fstream* dumpStream) {
  if (dumpStream->is_open()) {
    for(std::vector<ResultRow>::iterator rowIt = rows.begin(); rowIt != rows.end(); rowIt++) {
      ResultRow row = *rowIt;
      *dumpStream << row.streamId << " " << row.entityURL << " "<< row.score << " " << row.dateHour << " "  << row.modelName << " " << row.maxScore << "\n";
    }
  }
  else
    std::cout << "\n Could not dump to file :\n ";
}

#endif
