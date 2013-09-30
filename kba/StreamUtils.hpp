#ifndef STREAMUTILS_HPP
#define STREAMUTILS_HPP
#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"
#include <cstdio>

namespace streamcorpus {
  namespace utils {
    std::string getTitle(streamcorpus::StreamItem& streamItem);
    std::string getAnchor(streamcorpus::StreamItem& streamItem);
  }
}
#endif
