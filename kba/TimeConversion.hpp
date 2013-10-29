#ifndef TIMECONVERSION_HPP
#define TIMECONVERSION_HPP

#include <time.h>
#include <string>

namespace kba {
  namespace time {
    /**
     * This function returns the value according to current timze zone setting.
     * To return current time setting as per UTC/GMT. Set the time to UTC.
     * u must unset it to correct timezone at the end.
     * http://man7.org/linux/man-pages/man3/timegm.3.html
     */   
    time_t convertDateToTime(std::string kbaDate);
  }
}
#endif
