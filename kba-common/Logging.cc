#include "Logging.hpp"


std::ofstream Logger::LOG_FILE;
void Logger::LOGGER(std::string debugFile) {
  (Logger::LOG_FILE).open(debugFile);
}


void Logger::CLOSE_LOGGER() {
  (Logger::LOG_FILE).close();
}


