#include <db_cxx.h>
#include "TermDict.hpp"


namespace kba {
  namespace berkley {
    class CorpusDb {
    private:
      std::string _baseDir;
      Db* _termDB;
      Db* _corpusDB;
      DbEnv* _dbEnv;
      u_int32_t _envFlags;
    public:
      CorpusDb(std::string baseEnvDir);
      ~CorpusDb();
      void addTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value);
      void updateCorpusStat(kba::term::CorpusStatKey* key, kba::term::CorpusStatValue* value);
    };        
  }
}
