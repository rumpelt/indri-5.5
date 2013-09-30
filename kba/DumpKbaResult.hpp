#ifndef DUMPKBARESULT_HPP
#define DUMPKBARESULT_HPP
#include <string>
#include <cstdio>
#include <vector>

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
      ResultRow() : teamId("udel"), slot("NULL"), equivalent("-1"), byteRange("1-1") {};
    };

    //   extern std::vector<ResultRow> RESULTROWS;
    //    std::string _DUMPFILE;
    
    ResultRow makeCCRResultRow(std::string streamId, std::string entityURL, int score, std::string dateHour, short relevant=2, bool mention=true);
    void addToResultRows(std::string dumpFile, std::string streamId, std::string entityURL, int score, std::string dateHour, short relevant=2, bool mention=true);
          
    void flushToDumpFile(std::vector<ResultRow>& rows, std::string& dumpFile);
    void writeHeader(std::string dumpFile);
    std::string&  rowHeader(); 
    std::vector<ResultRow>&  resultRows(); 
  }
}
#endif
