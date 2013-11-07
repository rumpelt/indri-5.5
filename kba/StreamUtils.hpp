#ifndef STREAMUTILS_HPP
#define STREAMUTILS_HPP
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "ParsedStream.hpp"
#include <unordered_set>
#include <string>
#include <cstdio>

namespace streamcorpus {
  namespace utils {

    std::string getTitle(streamcorpus::StreamItem& streamItem);
    std::string getAnchor(streamcorpus::StreamItem& streamItem);
    kba::stream::ParsedStream* createParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopSet);

    /**
     * Create a light weight parse stream..Just contains the set of tokens. Just populated ParsedStream::tokenSet;
     */
    kba::stream::ParsedStream* createLightParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopSet);
    /**
     * For term freq and thus we can get anything.
     */
    kba::stream::ParsedStream* createMinimalParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopSet, std::set<std::string>& termsToFetch);
  }
}
#endif
