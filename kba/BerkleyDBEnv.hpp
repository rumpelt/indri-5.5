#ifndef BERKLEYDBENV_HPP
#define BERKLEYDBENV_HPP

#include <db_cxx.h>
#include "TermDict.hpp"
#include "boost/shared_ptr.hpp"

namespace kba {
  namespace berkley {
    class CorpusDb {
    private:
      std::string _baseDir;
      Db* _termDb;
      Db* _termTopicDb;
      Db* _corpusDb;
      Db* _topicDb; //Allows duplicate records with the topic name as the secondary key
      Db* _evalDb;
      DbEnv* _dbEnv;
      u_int32_t _envFlags;
    public:
      CorpusDb(std::string baseEnvDir, bool readOnly);
      /**
       *
       */
      CorpusDb(std::string baseEnvDir, std::string evalDbName);
      /**
       *Beware of closing this when you are using in threaded environment 
       */
      ~CorpusDb();
      void addTopicTerm(kba::term::TopicTermKey* key, kba::term::TopicTermValue* value);
      void addCorpusStat(kba::term::CorpusStat* corpusStat);
      void addTermStat(kba::term::TermStat* termStat);
      void addTopicStat(kba::term::TopicStat* topicStat);
      void addEvaluationData(kba::term::EvaluationData* evalData);

      std::vector<boost::shared_ptr<kba::term::EvaluationData> > getEvaluationData(std::string stream_id, std::string topic);
    };        
  }
}

#endif
