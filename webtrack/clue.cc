#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <unordered_set>
#include <assert.h>
#include <iostream>

#ifdef _cplusplus
extern "C" {
#endif
#include <libxml/tree.h>
#include <libxml/parser.h>
#ifdef _cplusplus
}
#endif
#include "boost/program_options/options_description.hpp"
#include "boost/program_options/variables_map.hpp"
#include "boost/program_options/parsers.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/ParsedDocument.hpp"
#include "indri/ScoredExtentResult.hpp"
#include "indri/SnippetBuilder.hpp"
#include "indri/MetadataPair.hpp"

#include "Passage.hpp"
#include "Tokenize.hpp"
#include "LanguageModel.hpp"


using namespace indri::api; 
namespace cmndOp = boost::program_options;

struct ClueResult {
  std::string tpcNum;
  std::string q0; //fixed
  std::string docId;
  int rank;
  float score;
  std::string runTag;
  ClueResult() : q0("Q0") {}
};
struct Query {
  std::string qnum;
  std::string query; // the actual query, for subqueries this might be empty
  std::string queryType;
  std::string description; // the actual description of the query
  std::vector<std::string> tokens;
  std::map<std::string, int> termFreq;
  std::vector<Query*> subquery;
  //  ~Query() {for_each(subquery.begin(), subquery.end(), delete)}
};

bool comparePsg(Passage* opsg, Passage* cpsg) {
  if(opsg->getScore() > cpsg->getScore()) 
    return true;
  return false;
}

std::vector<Query*> constructQuery(std::string queryFile) {
  LIBXML_TEST_VERSION
  xmlDoc* doc = 0;
  doc = xmlReadFile(queryFile.c_str(), NULL, 0); // DOM tree
  std::vector<Query*> querySet;
  if(doc == 0) {
    std::cout << "Could not parse xml topic file ";
    return querySet;
  }
  xmlNode* node = (xmlDocGetRootElement(doc))->children; // Sibling of the root 
  
  for( ;node; node = node->next) {
    std::string nodeName = (const char*)(node->name);
    //    std::cout << nodeName << "\n" ; 
    //<< (const char*)(node->_private) <<"\n";
    if(node->type == XML_ELEMENT_NODE && nodeName.compare("topic") == 0) {
      Query* q = new Query();
      xmlAttr* attr  = node->properties;      
      std::string attName = (const char*)(attr->name);
      //      std::cout << attName << "\n";
      std::string data = (const char*)((attr->children)->content);
      //std::cout << data << "\n";
      if(attName.compare("number") == 0)
        q->qnum = data;
      else
        q->queryType = data;

      attr = attr->next;
      attName = (const char*)(attr->name); 
      data = (const char*)((attr->children)->content);
      //     std::cout << attName << " : "<< data << "\n"; 
      if(attName.compare("type") == 0)
        q->queryType = data;
      else
        q->qnum = data;
      
      xmlNode* child = node->children;
      for(;child; child = child->next) {
        if(child->type != XML_ELEMENT_NODE)
          continue;
	std::string subName = (const char*)(child->name);
	//	std::cout << subName << "\n"; 	
        if(subName.compare("query") == 0) {
          std::string data = (const char*) ((child->children)->content);
          q->query = data;
	}
        else if(subName.compare("description") == 0) {
	  std::string data = (const char*) ((child->children)->content);
          q->description = data;
            
	}
        else if(subName.compare("subtopic") == 0) {
	  std::string data = (const char*) ((child->children)->content);
          Query* subq = new Query();
          subq->description = data;
          xmlAttr* subattr = child->properties;
	  std::string sAttrName = (const char*)(subattr->name);
          data = (const char*) ((subattr->children)->content);
          if(sAttrName.compare("number") == 0) {
            subq->qnum = data;
	  }
          else
            subq->queryType = data;
          
          subattr = subattr->next;
          sAttrName = (const char*)(subattr->name);
          data = (const char*)((subattr->children)->content);
 
          if(sAttrName.compare("number") == 0) {
            subq->qnum = data;
	  }
          else
            subq->queryType = data;
          (q->subquery).push_back(subq);
	  //  std::cout << " ==subquery== " << subq->description << " : " << subq->qnum << " : " << subq->queryType << "\n";
	}
      }
      //      std::cout << "Adding query :" << q->query << " : " << q->qnum << " : " << q->description << " : " << q->queryType  << "\n";
      querySet.push_back(q);
    }
  }
  xmlFreeDoc(doc);
  xmlCleanupParser();
  return querySet;
}

/**
 * Create IndexEnvironment. Can accept servername  as in 127.0.0.1:<port number> or localhost:<port> or hostname:<port>
 */
indri::api::QueryEnvironment* createIndexEnvironment(std::string server, std::vector<std::string> stopwords, bool remote) {
  indri::api::QueryEnvironment* qe = new indri::api::QueryEnvironment();
  if (remote)
    qe->addServer(server);
  else {
    qe->addIndex(server);
    qe->setMemory(2147483648); // 2GB = 1024* 1024* 1024 * 2
  }
  if(stopwords.size() > 0)
    qe->setStopwords(stopwords);
  return qe;
}

/**
 * the vector is sorted in decresing order of score
 */
std::vector<ClueResult> prepareResult(std::vector<Passage*> psgs, std::string qnum, std::string runTag) {
  std::vector<ClueResult> cresult;
  std::unordered_set<int> seenIds;
  int rank = 1;
  for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    if (seenIds.find(psg->getDocId()) == seenIds.end()) {
      ClueResult cr;
      cr.tpcNum = qnum;
      cr.docId = psg->getTrecId();
      cr.score = psg->getScore();
      cr.rank = rank;
      rank++;
      cr.score = psg->getScore(); 
      cr.runTag = runTag;
      cresult.push_back(cr);
    }
    seenIds.insert(psg->getDocId());
  }
  return cresult;
}

void dumpResult(std::vector<ClueResult> cresult, std::string dumpFile) {
  std::fstream dumpstream;
  dumpstream.open(dumpFile.c_str(), std::fstream::out| std::fstream::app);
  for (std::vector<ClueResult>::iterator clIt = cresult.begin(); clIt != cresult.end(); ++clIt) {
    ClueResult cl = *clIt;
    dumpstream << cl.tpcNum << " " << cl.q0 << " " << cl.docId << "  " <<cl.rank << " " << cl.score << " " << cl.runTag << "\n";
  }
  dumpstream.close();
}
/**
 * passageSz : number of tokens to be kept in a passage.
 * windowSz : number of floating tokens which will be shared across two consecutive passages
 * Responsibilty of caller to delete passages
 */
std::vector<Passage*> createPassage(std::string content, bool lower, std::unordered_set<std::string> stopwordSet, int passageSz, int windowSz, int docId) {
  assert(passgeSz > windowSz);
  std::vector<Passage*> psgs;
  std:vector<std::string> tokens = Tokenize::tokenize(content, lower, stopwordSet);
  std::vector<std::string> psgTokens;
  std::vector<std::string> windowTokens;
  int id=1;
  for(std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
    std::string term = *tokIt;
    if(psgTokens.size() >= passageSz) {
      Passage* psg = new Passage();
      psg->setPsgId(id++);
      psg->setDocId(docId);
      psg->setTerms(psgTokens);
      psgs.push_back(psg);
      psgTokens = windowTokens;
      windowTokens.clear();
    }
    psgTokens.push_back(term);
    windowTokens.push_back(term);
    if(windowTokens.size() >  windowSz)
      windowTokens.erase(windowTokens.begin());
  }
  if(psgTokens.size() > 0) {
    Passage* psg = new Passage();
    psg->setPsgId(id++);
    psg->setDocId(docId);
    psg->setTerms(psgTokens);
    psgs.push_back(psg);
  }
  return psgs;  
}

std::vector<ClueResult> reScore(QueryEnvironment* qe, Query* query, std::vector<ScoredExtentResult>& rslts, bool lower, std::unordered_set<std::string> stopSet, int passageSz, int windowSz) {
  std::vector<ParsedDocument*> pdocs = qe->documents(rslts); // caller has to delete result ParsedDocuments
  //  std::string trecField = "WARC-TREC-ID";
  //std::vector<std::string> trecids = qe->documentMetadata(rslts, trecField);

  std::vector<Passage*> allPsgs;
  std::map<std::string, long> collFreq;
  long collSz = qe->termCount();
  LanguageModel lm;
 if((query->tokens).size() <= 0) {
    std::vector<std::string> tokens = Tokenize::tokenize(query->query, lower, stopSet);
    for (std::vector<std::string>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
      (query->termFreq)[*tokIt]++;
      (query->tokens).push_back(*tokIt);
       collFreq.insert(std::pair<std::string, long>(*tokIt, qe->termCount(*tokIt)));
    }
  }

  int idx=0;
  for(std::vector<ParsedDocument*>::iterator pdIt = pdocs.begin(); pdIt != pdocs.end(); ++pdIt) {
    ParsedDocument* pdoc = *pdIt;
    std::string trecId;
    using namespace indri::utility;
    using namespace indri::parse;
    greedy_vector<indri::parse::MetadataPair> md = pdoc->metadata;
    for(greedy_vector<indri::parse::MetadataPair>::iterator it = md.begin(); it != md.end(); ++it)  {
      indri::parse::MetadataPair m = *it;
      std::string key = m.key;
      if(key.compare("warc") == 0) {
	std::string va = (const char*)(m.value);
        int spos = va.find("clueweb12");
        int epos = va.find("\n", spos);
	trecId = va.substr(spos, epos - spos -1);
      }
      if(key.compare("docno") == 0)
	std::cout << "docno" ;
    }

    ScoredExtentResult ser = rslts[idx];
    idx++;
    //    std::cout << "New Doc "  << "\n" << trecId << "\n";
    std::vector<Passage*> psgs = createPassage(pdoc->getContent(), lower, stopSet, passageSz, windowSz ,ser.document);
    for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
      Passage* psg = *psgIt;  
      psg->setTrecId(trecId);
      psg->crtTermFreq();
      float score = lm.score(query->tokens, psg, collFreq, collSz);
      std::vector<std::string> terms = psg->getTerms();
      for(std::vector<std::string>::iterator tkIt = (terms).begin(); tkIt != (terms).end(); ++tkIt) {
	std::cout <<" " << *tkIt;
      }
      std::cout << "\nPassage  " << score << "\n";
      
      psg->setScore(score);
      allPsgs.push_back(psg);
    }
  }

  std::sort(allPsgs.begin(), allPsgs.end(), comparePsg); 
  std::vector<ClueResult> cresult = prepareResult(allPsgs,  query->qnum, "test");
  for(std::vector<ParsedDocument*>::iterator pdIt = pdocs.begin(); pdIt != pdocs.end(); ++pdIt) 
    delete *pdIt;
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) 
    delete *psgIt;
  return cresult;
}

std::vector<indri::api::ScoredExtentResult> runQuery(indri::api::QueryEnvironment* qe, std::string query, int rsltSize) {
  using namespace indri::api;
  //  indri::api::QueryAnnotation* qrslt =  qe->runQuery(query, rsltSize);
  //  std::vector<indri::api::ScoredExtentResult> rslts = qrslt->getResults();
  std::vector<indri::api::ScoredExtentResult> rslts =  qe->runQuery(query, rsltSize);
  //  delete qrslt;
  return rslts;
  /**  
  indri::api::SnippetBuilder sb(false);
  std::vector<indri::api::ParsedDocument*> pdocs = qe->documents(rslts);
  int idx = 0;
  for(std::vector<ParsedDocument*>::iterator pdIt = pdocs.begin(); pdIt != pdocs.end(); ++pdIt) {
    ParsedDocument* pdoc = *pdIt;
    indri::api::ScoredExtentResult rslt = rslts[idx++];
    std::cout << sb.build(rslt.document, pdoc, qrslt) << "\n";

  }
  for(std::vector<ParsedDocument*>::iterator pdIt = pdocs.begin(); pdIt != pdocs.end(); ++pdIt)
    delete *pdIt;
  delete qrslt;  
  */
}

void performAdhocTask(std::string indexPath, std::string topicFile, std::string stopFile, std::string dumpFile) {
  std::vector<std::string> stopwords;
  std::unordered_set<std::string> stopwordSet = Tokenize::getStopSet(stopFile);
  std::cout << stopwordSet.size() << "\n";
  for(std::unordered_set<std::string>::iterator wordIt =  stopwordSet.begin(); wordIt != stopwordSet.end(); ++wordIt) {
    stopwords.push_back(*wordIt);
  }

  std::vector<Query*> querySet = constructQuery(topicFile);
  bool isRemoteServer=false;
  if (indexPath.find(":") != string::npos)
    isRemoteServer = true;
  indri::api::QueryEnvironment* qe = createIndexEnvironment(indexPath, stopwords , isRemoteServer);

  for(std::vector<Query*>::iterator qIt = querySet.begin(); qIt != querySet.end(); ++qIt) {
    Query* query = *qIt;
    std::vector<ScoredExtentResult> rslts =  runQuery(qe, query->query, 05);   
    std::cout << "Query ::" << query->query << "\n";
    std::vector<ClueResult> cresult = reScore(qe, query,  rslts, true, stopwordSet, 150, 50);
    dumpResult(cresult, dumpFile);    
  }
}

int main(int argc, char* argv[]) {
  std::string logFile;
  std::string stopFile;
  std::string indexName; // Can be remote repository or a server name
  std::string topicFile; // The query to be executed
  std::string dumpFile;
  cmndOp::options_description cmndDesc("Allowed command line options");
  cmndDesc.add_options()
    ("help", "the help message")
    ("log", cmndOp::value<std::string>(&logFile), "The log file to write message to")
    ("dfile", cmndOp::value<std::string>(&dumpFile), " The file to dump to ")
    ("index", cmndOp::value<std::string>(&indexName), "The Index dir/server to query against")
  ("stop-file,SF",cmndOp::value<std::string>(&stopFile)->default_value("../help/sql40stoplist"),"the file containing the stop words")
  ("qfile",cmndOp::value<std::string>(&topicFile)->default_value("/usa/arao/trec/trec-web/clueweb12/trec2013-topics.xml"), " The query file ");
  
  cmndOp::variables_map cmndMap;
  cmndOp::store(cmndOp::parse_command_line(argc, argv,cmndDesc), cmndMap);
  cmndOp::notify(cmndMap);  
  
  performAdhocTask(indexName, topicFile, stopFile, dumpFile);
  return 0;
}
