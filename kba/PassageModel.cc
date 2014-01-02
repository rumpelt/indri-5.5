#include "PassageModel.hpp"
#include "PassageUtility.hpp"

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

bool comparePsg(Passage* opsg, Passage* cpsg) {
  if(opsg->getScore() > cpsg->getScore()) 
    return true;
  return false;
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

Passage PassageModel::createPassage(std::vector<std::string>& terms, lemur::api::DOCID_T docId, bool discardJunk) {
  Passage psg;
  psg.setPsgId(1);
  psg.setDocId(docId);
  std::string junk = "[OOV]";
  for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt) {
    std::string term = *termIt;
    if(discardJunk && term.compare(junk) == 0)
      continue;
    psg.pushTerm(term);
  }
  return psg;
}

std::vector<std::string> PassageModel::constructDocFromVector(indri::api::DocumentVector* dv, bool discardJunk) {
  std::vector<std::string> doc;
  std::string junk = "[OOV]";
  for(size_t idx = 0; idx < dv->positions().size(); idx++) {
    int position = dv->positions()[idx];
    std::string term = dv->stems()[position];
    if(discardJunk && term.compare(junk) == 0)
      continue;
    doc.push_back(term);
  }
  return doc;
}

/**
 * passageSz : number of tokens to be kept in a passage.
 * windowSz : number of floating tokens which will be shared across two consecutive passages
 * Responsibilty of caller to delete passages
 */
std::vector<Passage*> PassageModel::createPassageFromDocumentVector(indri::api::DocumentVector* dv, bool discardJunk, const int passageSz, const int windowSz, unsigned long docId) {
  //  assert(passgeSz > windowSz);
  std::vector<Passage*> psgs;
  std::vector<std::string> psgTokens;
  std::vector<std::string> windowTokens;
  int id=1;
  std::string junk = "[OOV]";
  for(size_t idx=0; idx < dv->positions().size(); idx++) {
    int position = dv->positions()[idx];
    std::string term = dv->stems()[position];
    if(discardJunk && term.compare(junk) == 0)
      continue;
    //    std::cout << term << "\n";
    if(psgTokens.size() >= passageSz) {
      Passage* psg = new Passage();
      psg->setPsgId(id++);
      psg->setDocId(docId);
      psg->setTerms(psgTokens);
      psgs.push_back(psg);
      psgTokens.clear();
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

/**
 * passageSz : number of tokens to be kept in a passage.
 * windowSz : number of floating tokens which will be shared across two consecutive passages
 * Responsibilty of caller to delete passages
 */
std::vector<Passage*> PassageModel::createFixedWindowPassage(std::string content, bool lower, std::unordered_set<std::string> stopwordSet, const int passageSz, const int windowSz, unsigned long docId) {
  //  assert(passgeSz > windowSz);
  std::vector<Passage*> psgs;
  std:vector<std::string> tokens = Tokenize::whiteSpaceSplit(content, stopwordSet); // will stem also
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
      psgTokens.clear();
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

std::vector<Passage*> createPassageFromParsedDoc(ParsedDocument* pdoc, bool lower, std::unordered_set<std::string> stopwordSet, int passageSz, int windowSz, unsigned long docId) {
  using namespace indri::utility;
  assert(passgeSz > windowSz);
  std::vector<Passage*> psgs;
  greedy_vector<char*> tokens = pdoc->terms;
  std::vector<std::string> windowTokens;
  std::vector<std::string> psgTokens;
  int id=1;
  for(greedy_vector<char*>::iterator tokIt = tokens.begin(); tokIt != tokens.end(); ++tokIt) {
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

std::vector<ScoredExtentResult> PassageModel::intrpMaxPsgScoringCosineHom(QueryEnvironment* qe, Query* query, std::vector<ScoredExtentResult>& rslts, const bool lower, std::unordered_set<std::string> stopSet, const int passageSz, const int windowSz) {
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<ScoredExtentResult>::iterator srIt = rslts.begin(); srIt != rslts.end(); ++srIt) {
    docIds.push_back((*srIt).document);
  }
  std::vector<DocumentVector*> dvVector = qe->documentVectors(docIds);
  
  std::string trecField = "docno";
  std::vector<std::string> trecids = qe->documentMetadata(rslts, trecField);

  std::map<std::string, unsigned long> collFreq;
  std::vector<std::string> qTokens = Tokenize::whiteSpaceSplit(query->query, stopSet);  
  for(std::vector<std::string>::iterator qIt = qTokens.begin(); qIt != qTokens.end(); ++qIt) {
    collFreq.insert(std::pair<std::string, unsigned long>(*qIt, qe->termCount(*qIt)));
  }
  
  std::map<std::string, ScoredExtentResult> srMap;
  LanguageModelPsg lm;
  unsigned long collectionSize = qe->termCount();
  
  std::vector<Passage*> allPsgs;
  int idx = 0;
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt, ++idx) {
    DocumentVector* dv = *dvIt;
    std::string trecId = trecids[idx];
    ScoredExtentResult ser = rslts[idx];
    std::vector<std::string> docContent = constructDocFromVector(dv, true);
    Passage motherPassage = createPassage(docContent, ser.document, true);
    motherPassage.crtTermFreq();
    
    std::vector<Passage*> psgs = createPassageFromDocumentVector(dv, true, passageSz, windowSz ,ser.document);
   
    for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
      Passage* psg = *psgIt;
      psg->crtTermFreq();
      psg->setTrecId(trecId);
    } 
 
    //    float homogeniety_my = passageutil::docPsgHomogeniety(psgs, &motherPassage, qe, true);
    float homogeniety = passageutil::psgCosineSimilarity(psgs, &motherPassage, qe);
    //std::cout << "My homg " << homogeniety_my << " homg " <<  homogeniety << "\n";

    for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
      Passage* psg = *psgIt;
      float score = lm.score(qTokens, psg, collFreq, collectionSize);
      score = (homogeniety * ser.score ) + (1 - homogeniety) * score;
      psg->setScore(score);
      allPsgs.push_back(psg);
      srMap.insert(std::pair<std::string, ScoredExtentResult>(trecId, ser));
    }
  }

  std::sort(allPsgs.begin(), allPsgs.end(), comparePsg); 
  std::unordered_set<std::string> seenIds;
  std::vector<ScoredExtentResult> srs;
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    /**
    std::vector<std::string> terms = psg->getTerms();
    for(std::vector<std::string>::iterator termIt = (psg->getTerms()).begin(); termIt != (psg->getTerms()).end(); ++termIt)
      *termIt;
    std::cout << "\n" <<psg->getScore() << " " << psg->getTrecId() << "\n";
    */
    if(seenIds.find(psg->getTrecId()) == seenIds.end()) {
      ScoredExtentResult sr = srMap[psg->getTrecId()];
      sr.score = psg->getScore();
      srs.push_back(sr);
    }
    seenIds.insert(psg->getTrecId());
  }
  
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt)
    delete *dvIt;
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) {
    delete(*psgIt); 
  }

  return srs; 
}

std::vector<ScoredExtentResult> PassageModel::intrpMaxPsgScoringLengthHom(QueryEnvironment* qe, Query* query, std::vector<ScoredExtentResult>& rslts, const bool lower, std::unordered_set<std::string> stopSet, const int passageSz, const int windowSz)
 {
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<ScoredExtentResult>::iterator srIt = rslts.begin(); srIt != rslts.end(); ++srIt) {
    docIds.push_back((*srIt).document);
  }
  std::vector<DocumentVector*> dvVector = qe->documentVectors(docIds);
  
  std::vector<std::string> qTokens = Tokenize::whiteSpaceSplit(query->query, stopSet); 
  std::string trecField = "docno";
  std::vector<std::string> trecids = qe->documentMetadata(rslts, trecField);
  std::map<std::string, unsigned long> collFreq;
  for(std::vector<std::string>::iterator qIt = qTokens.begin(); qIt != qTokens.end(); ++qIt) {
    collFreq.insert(std::pair<std::string, unsigned long>(*qIt, qe->termCount(*qIt)));
  }

  std::map<std::string, int> docLengthMap;
  int maxDocLength = 0;
  int minDocLength = 999999;
  
  int pdx =0;
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt, ++pdx) {
    DocumentVector* dv = *dvIt;
    int docLength = dv->positions().size(); // I am counting the junk characters also
    //std::cout 
    if (docLength > maxDocLength)
      maxDocLength = docLength;
    else if (docLength != 0 && docLength < minDocLength)
      minDocLength = docLength;
    docLengthMap.insert(std::pair<std::string, int>(trecids[pdx], docLength));
  }

  assert(minDocLength > 0);

  if(minDocLength >= maxDocLength) {
    maxDocLength = 2 * passageSz;
    std::cout << "Max doc length set " << maxDocLength << "\n";
  }

  
  std::map<std::string, ScoredExtentResult> srMap;
  LanguageModelPsg lm;
  unsigned long collectionSize = qe->termCount();
  std::vector<Passage*> allPsgs;
  int idx = 0;

  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt, ++idx) {
    DocumentVector* dv = *dvIt;
    std::string trecId = trecids[idx];
    int docLength = docLengthMap[trecId];
    float homogeniety = passageutil::lenHomogeniety(minDocLength, maxDocLength, docLength);
    ScoredExtentResult ser = rslts[idx];
    std::vector<Passage*> psgs = createPassageFromDocumentVector(dv, true, passageSz, windowSz ,ser.document);
    for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
      Passage* psg = *psgIt;  
      psg->crtTermFreq();
      psg->setTrecId(trecId);
      float score = lm.score(qTokens, psg, collFreq, collectionSize);
      score = ( homogeniety * ser.score ) + (1 - homogeniety) * score;
      psg->setScore(score);
      //      std::cout << "Min len " << minDocLength << " Max " << maxDocLength << " Length " << docLength << " homo " << homogeniety << " score " << ser.score << " total " << score << "\n";
      allPsgs.push_back(psg);
      srMap.insert(std::pair<std::string, ScoredExtentResult>(trecId, ser));
    }
  }
  
  std::sort(allPsgs.begin(), allPsgs.end(), comparePsg); 
  std::unordered_set<std::string> seenIds;
  std::vector<ScoredExtentResult> srs;
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    /**
    std::vector<std::string> terms = psg->getTerms();
    for(std::vector<std::string>::iterator termIt = (psg->getTerms()).begin(); termIt != (psg->getTerms()).end(); ++termIt)
      *termIt;
    std::cout << "\n" <<psg->getScore() << " " << psg->getTrecId() << "\n";
    */
    if(seenIds.find(psg->getTrecId()) == seenIds.end()) {
      ScoredExtentResult sr = srMap[psg->getTrecId()];
      sr.score = psg->getScore();
      srs.push_back(sr);
    }
    seenIds.insert(psg->getTrecId());
  }
  
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) 
    delete *psgIt;
  
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt) 
    delete *dvIt;
  return srs;  
}

std::vector<ScoredExtentResult> PassageModel::maxPsgScoring(QueryEnvironment* qe, Query* query, std::vector<ScoredExtentResult>& rslts, const bool lower, std::unordered_set<std::string> stopSet, const int passageSz, const int windowSz) {
   // = qe->documents(rslts); // caller has to delete result ParsedDocuments
  //std::vector<ParsedDocument*> pdocs = qe->documents(rslts); // caller has to delete result ParsedDocuments
  std::vector<lemur::api::DOCID_T> docIds;
  for(std::vector<ScoredExtentResult>::iterator srIt = rslts.begin(); srIt != rslts.end(); ++srIt) {
    docIds.push_back((*srIt).document);
  }
  std::vector<DocumentVector*> dvVector = qe->documentVectors(docIds);
  
  std::vector<std::string> qTokens = Tokenize::whiteSpaceSplit(query->query, stopSet); 
  std::map<std::string, unsigned long> collFreq;
  for(std::vector<std::string>::iterator qIt = qTokens.begin(); qIt != qTokens.end(); ++qIt) {
    collFreq.insert(std::pair<std::string, unsigned long>(*qIt, qe->termCount(*qIt)));
  }

  std::vector<Passage*> allPsgs;
  int idx = 0;
  const std::string trecField = "docno";
  std::vector<std::string> trecids = qe->documentMetadata(rslts, trecField);
  std::map<std::string, ScoredExtentResult> srMap;
  LanguageModelPsg lm;
  unsigned long collectionSize = qe->termCount();
  assert(collectionSize > 0);
  //  std::cout << " doc vector " << dvVector.size();
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt, ++idx) {
    DocumentVector* dv = *dvIt;
    ScoredExtentResult ser = rslts[idx];
    std::string trecId = trecids[idx];
    std::vector<Passage*> psgs = createPassageFromDocumentVector(dv, true, passageSz, windowSz ,ser.document);
    
    for(std::vector<Passage*>::iterator psgIt = psgs.begin(); psgIt != psgs.end(); ++psgIt) {
      Passage* psg = *psgIt;  
      //std::cout << (psg->getTerms()).size() << "\n";
      psg->crtTermFreq();
      psg->setTrecId(trecId);
      float score = lm.score(qTokens, psg, collFreq, collectionSize);
      psg->setScore(score);

      /**
      std::vector<std::string> terms = psg->getTerms();
      for(std::vector<std::string>::iterator termIt = terms.begin(); termIt != terms.end(); ++termIt)
	std::cout << *termIt << " ";
      std::cout << "\n";
      std::cout << " Score : " << score << "\n";
      */
      allPsgs.push_back(psg);
      srMap.insert(std::pair<std::string, ScoredExtentResult>(trecId, ser));
    }
  }

  std::sort(allPsgs.begin(), allPsgs.end(), comparePsg); 
  std::unordered_set<std::string> seenIds;
  std::vector<ScoredExtentResult> srs;
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) {
    Passage* psg = *psgIt;
    /**
    std::vector<std::string> terms = psg->getTerms();
    for(std::vector<std::string>::iterator termIt = (psg->getTerms()).begin(); termIt != (psg->getTerms()).end(); ++termIt)
      *termIt;
    std::cout << "\n" <<psg->getScore() << " " << psg->getTrecId() << "\n";
    */
    if(seenIds.find(psg->getTrecId()) == seenIds.end()) {
      ScoredExtentResult sr = srMap[psg->getTrecId()];
      sr.score = psg->getScore();
      srs.push_back(sr);
    }
    seenIds.insert(psg->getTrecId());
  }
  
  for(std::vector<Passage*>::iterator psgIt = allPsgs.begin(); psgIt != allPsgs.end(); ++psgIt) 
    delete *psgIt;
  
  for(std::vector<DocumentVector*>::iterator dvIt = dvVector.begin(); dvIt != dvVector.end(); ++dvIt) 
    delete *dvIt;
  return srs;
}


std::vector<indri::api::ScoredExtentResult> runQuery(indri::api::QueryEnvironment* qe, std::string query, int rsltSize) {
  using namespace indri::api;
  //  indri::api::QueryAnnotation* qrslt =  qe->runQuery(query, rsltSize);
  //  std::vector<indri::api::ScoredExtentResult> rslts = qrslt->getResults();
  query = "#combine( "+ query + ")";
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

/**
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
*/

int mainRun(int argc, char* argv[]) {
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
  
  //  performAdhocTask(indexName, topicFile, stopFile, dumpFile);
  return 0;
}
