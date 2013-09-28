#include "StreamUtils.hpp"
/*
std::string streamcorpus::utils::getTitle(StreamItem& streamItem) {
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
*/
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

