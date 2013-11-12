#ifndef STREAMUTILS_HPP
#define STREAMUTILS_HPP
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include "ParsedStream.hpp"
#include <unordered_set>
#include <string>
#include <cstdio>
#include "stdexcept"
#include "Tokenize.hpp"


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

inline std::string streamcorpus::utils::getAnchor(streamcorpus::StreamItem& streamItem) {
  std::string anchor;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("anchor0");
    anchor = content.raw;
    return anchor;
  }
  catch(const std::out_of_range& orexpt) {
    return anchor;
  }
}

inline std::string streamcorpus::utils::getTitle(streamcorpus::StreamItem& streamItem) {
  std::string title;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("title");
    title  = content.raw;
    return title;
  }
  catch(const std::out_of_range& orexpt) {
    return title;
  }
}

inline kba::stream::ParsedStream* streamcorpus::utils::createMinimalParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopwords, std::set<std::string>& termsToFetch) {
  std::string title = streamcorpus::utils::getTitle(*streamItem);
  std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
  std::string body = (streamItem->body).clean_visible;
  std::string fullContent = title + anchor + body;
  std::vector<std::string> tokens = Tokenize::tokenize(fullContent);
  tokens = Tokenize::toLower(tokens); 
  tokens = Tokenize::filterStopWords(tokens, stopwords);
  kba::stream::ParsedStream *parsedStream = new kba::stream::ParsedStream(tokens.size());
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;    
    if(termsToFetch.find(token) == termsToFetch.end()) 
      continue;
    (parsedStream->tokenFreq)[token]++;
  }
  return parsedStream;
}

#endif
