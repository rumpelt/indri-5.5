#ifndef BERKLEYDBENV_HPP
#define BERKLEYDBENV_HPP


#ifdef _cplusplus
extern "C" { 
#endif
  #include <db.h>
#ifdef _cplusplus
}
#endif

#include "TermDict.hpp"
#include "boost/shared_ptr.hpp"

namespace kba {
  namespace berkley {
    class CorpusDb {
    private:
      std::string _baseDir;
      DB_ENV* _dbEnv;
      DB* _termDb;
      DB* _termTopicDb;
      DB* _corpusDb;
      DB* _topicDb; //Allows duplicate records with the topic name as the secondary key
      DB* _evalDb;
      
      u_int32_t _envFlags;
    public:
      CorpusDb(std::string baseEnvDir, bool readOnly);
      /**
       *
       */
      CorpusDb(std::string baseEnvDir, std::string evalDbName);
      ~CorpusDb();
      void addTopicTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value);
      void addCorpusStat(kba::term::CorpusStat* corpusStat);
      void addTermStat(kba::term::TermStat* termStat);
      void addTopicStat(kba::term::TopicStat* topicStat);
      void addEvaluationData(kba::term::EvaluationData* evalData);
      void init(std::string, std::string);
      std::vector<boost::shared_ptr<kba::term::EvaluationData> > getEvaluationData(std::string stream_id, std::string topic);
    };        
  }
}

#endif
