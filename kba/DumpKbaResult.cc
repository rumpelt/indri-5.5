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


void kba::dump::addToResultRows(std::string streamId, std::string entityURL, int score, std::string dateHour, short relevance, bool mention) {
  ResultRow row = kba::dump::makeCCRResultRow(streamId, entityURL, score, dateHour,relevance, mention);
  kba::dump::RESULTROWS.push_back(row);
  if(kba::dump::RESULTROWS.size() > 100) {
    kba::dump::flushToDumpFile(kba::dump::RESULTROWS);
    kba::dump::RESULTROWS.clear();
  }
}

/**
 * access must be synchronized
 */
void kba::dump::flushToDumpFile(std::vector<ResultRow>& rows) {
  std::fstream dumpstream;
  dumpstream.open(kba::dump::_DUMPFILE.c_str(), std::fstream::out| std::fstream::app);
  if (dumpstream.is_open()) {
    for(std::vector<ResultRow>::iterator rowIt = rows.begin(); rowIt != rows.end(); rowIt++) {
      ResultRow row = *rowIt;
      dumpstream << row.teamId << " " << row.systemId << " " << row.streamId << " " << row.entityURL << " "<< row.score << " " << row.relevant << " " << row.mention << " " << row.dateHour << " " << row.slot << " " << row.equivalent << " " << row.byteRange << "\n";
    }
    dumpstream.close();
  }
  else
    std::cout << "\n Could not dump to file : " << kba::dump::_DUMPFILE << "\n";
}
