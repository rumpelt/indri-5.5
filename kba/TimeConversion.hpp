#ifndef TIMECONVERSION_HPP
#define TIMECONVERSION_HPP

#include <time.h>
#include <string>

namespace kba {
  namespace time {
    /**
     * Convert a human readable date to long format.
     * this is for kba where direcotries are date. eg. 2011-10-31 to time_t data
     */
    time_t convertDateToTime(std::string kbaDate);
  }
}
#endif
