#include "DumpKbaResult.hpp"
#include <iostream>
#include <fstream>


kba::dump::ResultRow kba::dump::makeCCRResultRow(std::string streamId, std::string entityURL, int score, std::string dateHour, short relevant, bool mention ) {
  ResultRow resultrow;
  resultrow.streamId = streamId;
  resultrow.entityURL = entityURL;
  resultrow.score = score;
  resultrow.dateHour = dateHour;
  resultrow.mention = mention;
  resultrow.relevant = relevant;
  return resultrow;
}


void kba::dump::addToResultRows(std::vector<kba::dump::ResultRow>* rows, std::string fileName, std::string streamId, std::string entityURL, int score, std::string dateHour, short relevance, bool mention) {
  ResultRow row = kba::dump::makeCCRResultRow(streamId, entityURL, score, dateHour,relevance, mention);

  rows->push_back(row);
  
}
/**
 * should be synchronized if you are writing multiple times.
 */
void kba::dump::writeHeader(std::string dumpFile) {
  std::fstream dumpstream;
  dumpstream.open(dumpFile.c_str(), std::fstream::out| std::fstream::app);
  if(dumpstream.is_open()) {
    dumpstream << kba::dump::rowHeader() << "\n";
    dumpstream.close();
  }
}

/**
 * access must be synchronized
 */
void kba::dump::flushToDumpFile(std::vector<ResultRow>& rows, std::fstream* dumpStream) {

  if (dumpStream->is_open()) {
    for(std::vector<ResultRow>::iterator rowIt = rows.begin(); rowIt != rows.end(); rowIt++) {
      ResultRow row = *rowIt;
      //      std::cout << "Row id : " << row.streamId << "\n";
      *dumpStream << row.teamId << " " << row.systemId << " " << row.streamId << " " << row.entityURL << " "<< row.score << " " << row.relevant << " " << row.mention << " " << row.dateHour << " " << row.slot << " " << row.equivalent << " " << row.byteRange << "\n";
    }
    dumpStream->flush();
  }
  else
    std::cout << "\n Could not dump to file :\n ";
}

std::string&  kba::dump::rowHeader() {
  static std::string rowheader = "#{\"run_type\": \"automatic\", \"poc_email\": \"ashwani@udel.edu\", \"team_id\": \"UdelCis\", \"topic_set_id\": \"kba-2013-ccr-and-ssf\", \"corpus_id\": \"kba-streamcorpus-2013-v0_2_0\", \"$schema\": \"http://trec-kba.org/schemas/v1.1/filter-run.json\", \"team_name\": \"BlueHen\", \"system_description_short\": \"Not for the kab run yet\", \"system_description\": \"Not for kba run yet\", \"task_id\": \"kba-ccr-2013\", \"poc_name\": \"UDEL CIS-Ashwani\",  \"system_id\": \"Udelcis\"}";
  return rowheader;
}

std::vector<kba::dump::ResultRow> kba::dump::resultRows() {
  std::vector<kba::dump::ResultRow> rowscollection;
  return rowscollection;
}
