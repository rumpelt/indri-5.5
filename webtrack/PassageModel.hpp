#ifndef PASSAGEMODEL_HPP
#define PASSAGEMODEL_HPP

#include <vector>
#include <unordered_set>
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <unordered_set>
#include <assert.h>
#include <iostream>

#ifdef _cplusplus
extern "C" {
#endif
#include <libxml/tree.h>
#include <libxml/parser.h>
#ifdef _cplusplus
}
#endif

#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/ScoredExtentResult.hpp"
#include "indri/SnippetBuilder.hpp"
#include "indri/MetadataPair.hpp"
#include "indri/TagExtent.hpp"
#include "Passage.hpp"
#include "PassageModel.hpp"
#include "Tokenize.hpp"
#include "LanguageModel.hpp"
#include "Query.hpp"

namespace PassageModel {
  std::vector<Passage*> createFixedWindowPassage(std::string content, bool lower, std::unordered_set<std::string> stopwordSet, const int passageSz, const int windowSz, unsigned long docId);


std::vector<indri::api::ScoredExtentResult> maxPsgScoring(indri::api::QueryEnvironment* qe, Query* query, std::vector<indri::api::ScoredExtentResult>& rslts, const bool lower, std::unordered_set<std::string> stopSet, const int passageSz, const int windowSz);
};
#endif