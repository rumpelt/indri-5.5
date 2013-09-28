#include <string>
#include <cstdio>
#include <vector>

namespace kba {
  namespace dump {
    const std::string ResultHeader = "#{\"run_type\": \"automatic\", \"poc_email\": \"ashwani@udel.edu\", \"team_id\": \"UdelCis\", \"topic_set_id\": \"kba-2013-ccr-and-ssf\", \"corpus_id\": \"kba-streamcorpus-2013-v0_2_0\", \"$schema\": \"http://trec-kba.org/schemas/v1.1/filter-run.json\", \"team_name\": \"BlueHen\", \"system_description_short\": \"Not for the kab run yet\", \"system_description\": \"Not for kba run yet\", \"task_id\": \"kba-ccr-2013\", \"poc_name\": \"UDEL CIS-Ashwani\",  \"system_id\": \"Udelcis\"}";

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

    std::vector<ResultRow> RESULTROWS;
    std::string _DUMPFILE;
    
    ResultRow makeCCRResultRow(std::string streamId, std::string entityURL, int score, std::string dateHour, short relevant=2, bool mention=true);
    void addToResultRows(std::string streamId, std::string entityURL, int score, std::string dateHour, short relevant=2, bool mention=true);
    
    void flushToDumpFile(std::vector<ResultRow>& rows);
    
  }
  
  
}
