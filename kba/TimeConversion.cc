
#include <cassert>
#include "TimeConversion.hpp"
#include <stdio.h>
#include <iostream>

/**
 * This function returns the value according to current timze zone setting.
 * To return current time setting as per UTC/GMT. Set the time to UTC.
 * u must unset it to correct timezone at the end.
 * http://man7.org/linux/man-pages/man3/timegm.3.html
 */
time_t kba::time::convertDateToTime(std::string kbaDate) {
  struct tm timeStruct;
  size_t yearPos = kbaDate.find("-");
  size_t monthPos = kbaDate.find("-", yearPos + 1);
  //std::cout << "year " << kbaDate.substr(0, yearPos) << " month " << kbaDate.substr(yearPos+1, monthPos - yearPos -1) << " day " << kbaDate.substr(monthPos+1) << "\n";
  timeStruct.tm_year = std::strtol(kbaDate.substr(0, yearPos).c_str(), NULL,10) - 1900;
  timeStruct.tm_mon = std::strtol(kbaDate.substr(yearPos+1, monthPos - yearPos -1).c_str(),NULL, 10) - 1;
  timeStruct.tm_mday =  std::strtol(kbaDate.substr(monthPos+1).c_str(),NULL,10);
  timeStruct.tm_sec = 00;
  timeStruct.tm_hour = 0;
  timeStruct.tm_min = 0;
  timeStruct.tm_isdst = 0;
  time_t convertedTime = mktime(&timeStruct);
  assert(convertedTime != -1);
  return convertedTime;
}
