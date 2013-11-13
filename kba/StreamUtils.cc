#include "StreamUtils.hpp"

kba::stream::ParsedStream* streamcorpus::utils::createParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopwords) {
  std::string title = streamcorpus::utils::getTitle(*streamItem);
  std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
  std::string body = (streamItem->body).clean_visible;
  std::string fullContent = title + anchor + body;
  std::vector<std::string> tokens = Tokenize::tokenize(fullContent, true, stopwords);
  kba::stream::ParsedStream *parsedStream = new kba::stream::ParsedStream(tokens.size());
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    (parsedStream->tokenSet).insert(token);
    (parsedStream->tokens).push_back(token);
    try {
      (parsedStream->tokenFreq)[token]++;
    }
    catch(std::out_of_range& oor) {
    }
  }
  parsedStream->size = (parsedStream->tokens).size();
  return parsedStream;
}



kba::stream::ParsedStream* streamcorpus::utils::createLightParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopwords) {
  std::string title = streamcorpus::utils::getTitle(*streamItem);
  std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
  std::string body = (streamItem->body).clean_visible;
  std::string fullContent = title + anchor + body;
  std::vector<std::string> tokens = Tokenize::tokenize(fullContent, true, stopwords);
  kba::stream::ParsedStream *parsedStream = new kba::stream::ParsedStream(tokens.size());
  int size = 0;
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    (parsedStream->tokenSet).insert(token);
    ++size;
  }
  return parsedStream;
}
