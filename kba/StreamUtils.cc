#include "StreamUtils.hpp"
#include "stdexcept"
#include "Tokenize.hpp"

std::string streamcorpus::utils::getTitle(streamcorpus::StreamItem& streamItem) {
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

std::string streamcorpus::utils::getAnchor(streamcorpus::StreamItem& streamItem) {
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


kba::stream::ParsedStream* streamcorpus::utils::createParsedStream(streamcorpus::StreamItem* streamItem, std::unordered_set<std::string>& stopwords) {
  std::string title = streamcorpus::utils::getTitle(*streamItem);
  std::string anchor = streamcorpus::utils::getAnchor(*streamItem);
  std::string body = (streamItem->body).clean_visible;
  std::string fullContent = title + anchor + body;
  std::vector<std::string> tokens = Tokenize::tokenize(fullContent);
  tokens = Tokenize::toLower(tokens); 
  tokens = Tokenize::filterStopWords(tokens, stopwords);
  kba::stream::ParsedStream *parsedStream = new kba::stream::ParsedStream();
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    (parsedStream->tokenSet).insert(token);
    (parsedStream->tokens).push_back(token);
    try {
      int value = (parsedStream->tokenFreq).at(token);
      value = value + 1;
      (parsedStream->tokenFreq).erase(token);
      (parsedStream->tokenFreq).insert(std::pair<std::string, int>(token, value));
    }
    catch(std::out_of_range& oor) {
      (parsedStream->tokenFreq).insert(std::pair<std::string, int>(token, 1));
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
  std::vector<std::string> tokens = Tokenize::tokenize(fullContent);
  tokens = Tokenize::toLower(tokens); 
  tokens = Tokenize::filterStopWords(tokens, stopwords);
  kba::stream::ParsedStream *parsedStream = new kba::stream::ParsedStream();
  int size = 0;
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); tokIt++) {
    std::string token = *tokIt;
    (parsedStream->tokenSet).insert(token);
    ++size;
  }
  parsedStream->size = size;
  return parsedStream;
}
