#include "DumpKbaResult.hpp"
#include <iostream>




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


std::string&  kba::dump::rowHeader() {
  static std::string rowheader = "#{\"run_type\": \"automatic\", \"poc_email\": \"ashwani@udel.edu\", \"team_id\": \"UdelCis\", \"topic_set_id\": \"kba-2013-ccr-and-ssf\", \"corpus_id\": \"kba-streamcorpus-2013-v0_2_0\", \"$schema\": \"http://trec-kba.org/schemas/v1.1/filter-run.json\", \"team_name\": \"BlueHen\", \"system_description_short\": \"Not for the kab run yet\", \"system_description\": \"Not for kba run yet\", \"task_id\": \"kba-ccr-2013\", \"poc_name\": \"UDEL CIS-Ashwani\",  \"system_id\": \"Udelcis\"}";
  return rowheader;
}

std::vector<kba::dump::ResultRow> kba::dump::resultRows() {
  std::vector<kba::dump::ResultRow> rowscollection;
  return rowscollection;
}
