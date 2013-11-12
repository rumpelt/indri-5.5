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
      std::string teamId;
      std::string systemId;
      std::string streamId;
      std::string entityURL;
      int score;
      short relevant; // can be -1,0,1,2
      bool mention; // can be 0 or 1
      std::string dateHour; // the directory under which ResultRow for this stream was found
      std::string slot; // To be used for the ssf runs
      std::string equivalent; // The equivalence class to be used for the SSF run.
      std::string byteRange; // The byte range to be used for the SSF run
      std::string modelName;
      ResultRow() : teamId("udel"), systemId("CCR-ASD"),slot("NULL"), equivalent("-1"), byteRange("0-0") {};
    };

    //   extern std::vector<ResultRow> RESULTROWS;
    //    std::string _DUMPFILE;
    
    ResultRow makeCCRResultRow(std::string streamId, std::string entityURL, int score, std::string dateHour, std::string modelName, short relevant=2, bool mention=true);
          
    void flushToDumpFile(std::vector<ResultRow>& rows, std::fstream* dumpStream);
    void writeHeader(std::string dumpFile);
    std::string&  rowHeader(); 
    std::vector<ResultRow>  resultRows(); 
  }
}

inline kba::dump::ResultRow kba::dump::makeCCRResultRow(std::string streamId, std::string entityURL, int score, std::string dateHour, std::string modelName, short relevant, bool mention ) {
  ResultRow resultrow;
  resultrow.streamId = streamId;
  resultrow.entityURL = entityURL;
  resultrow.score = score;
  resultrow.dateHour = dateHour;
  resultrow.mention = mention;
  resultrow.relevant = relevant;
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
      //      std::cout << "Row id : " << row.streamId << "\n";
      *dumpStream << row.teamId << " " << row.systemId << " " << row.streamId << " " << row.entityURL << " "<< row.score << " " << row.relevant << " " << row.mention << " " << row.dateHour << " " << row.slot << " " << row.equivalent << " " << row.byteRange << " " << row.modelName <<"\n";
    }
    //    dumpStream->flush();
  }
  else
    std::cout << "\n Could not dump to file :\n ";
}

#endif
