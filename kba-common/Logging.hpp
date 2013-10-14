#include <fstream>  
#include <cstdio>
#include <string>
class Logger {
private:
    
public:
    static std::ofstream LOG_FILE;    
    static void LOGGER(std::string fileName);
    static void LOG_MSG(std::string className, std::string method, std::string msg);
    static void CLOSE_LOGGER();
};

inline void Logger::LOG_MSG(std::string className, std::string method, std::string msg) {
  (Logger::LOG_FILE) << "class: " <<  className << " : method: " << method << " :msg: " << msg << "\n";
  (Logger::LOG_FILE).flush(); 
}
