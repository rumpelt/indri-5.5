
#include <cassert>
#include "TimeConversion.hpp"

time_t kba::time::convertDateToTime(std::string kbaDate) {
  struct tm timeStruct;
  size_t yearPos = kbaDate.find("-");
  size_t monthPos = kbaDate.find("-", yearPos + 1);
  timeStruct.tm_year = std::strtol(kbaDate.substr(0, yearPos).c_str(), NULL,10) - 1900;
  timeStruct.tm_mon = std::strtol(kbaDate.substr(yearPos+1, monthPos - yearPos -1).c_str(),NULL, 10) - 1;
  timeStruct.tm_mday =  std::strtol(kbaDate.substr(monthPos+1).c_str(),NULL,10);
  timeStruct.tm_sec = 0;
  timeStruct.tm_hour = 0;
  timeStruct.tm_min = 0;
  time_t convertedTime = mktime(&timeStruct);
  assert(convertedTime != -1);
  return convertedTime;
}
