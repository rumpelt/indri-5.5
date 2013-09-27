
/**
 * Ashwani: implementation for the RDF Parsing.
 */

#include "RDFParser.hpp"
#include <vector>

#ifdef _cplusplus
extern "C" {
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _cplusplus
}
#endif

#include "Tokenize.hpp"
  

namespace cmdArg = boost::program_options;



static void generateAliases(RDFParser rdfparser, std::string fileName, std::string stopFileName) {
  //  librdf_parser* parser = rdfparser.initParser("ntriples");
  librdf_stream* stream = rdfparser.returnStreamFromRDFFile(rdfparser.getParser(), fileName, NULL);
  std::set<std::string> stopSet = Tokenize::getStopSet(stopFileName);
  if(stopSet.size() <= 0)
    std::cout << "cout not creat the stopSet \n";
  
  const char* unigram_string = "http://dbpedia.org/unigramtoken";
  librdf_uri* uniPredUri = librdf_new_uri(rdfparser.getWorld(), (const unsigned char*)unigram_string);
  librdf_node* uniPredNode = librdf_new_node_from_uri(rdfparser.getWorld(), uniPredUri);
 
  const char* bigram_string = "http://dbpedia.org/bigramtoken";
  librdf_uri* biPredUri = librdf_new_uri(rdfparser.getWorld(), (const unsigned char*)bigram_string);
  librdf_node* biPredNode = librdf_new_node_from_uri(rdfparser.getWorld(), biPredUri);
 
  const char* trigram_string = "http://dbpedia.org/trigramtoken";
  librdf_uri* triPredUri = librdf_new_uri(rdfparser.getWorld(), (const unsigned char*)trigram_string);
  librdf_node* triPredNode = librdf_new_node_from_uri(rdfparser.getWorld(), triPredUri);
 
 
  while(librdf_stream_end(stream) == 0) {
    librdf_statement* statement = librdf_stream_get_object(stream); // this call return a shared pointer and so we need not worry about it as we use this only in this scope
    
    librdf_node* subject = librdf_statement_get_subject(statement);   //returns a shared pointer  and must be copier by caller ifneeded and so we must make a duplicate
 
    librdf_node* object = librdf_statement_get_object(statement); // this again must be coppied if wer are goint to use it
    if (librdf_node_is_literal(object)) {
      std::string nodeText = rdfparser.convertNodeToString(object);
      //      std::cout << "Entity : "<< nodeText <<"\n";
      std::vector<std::string> unigrams = Tokenize::tokenize(nodeText);
      //      std::cout << "toeknized " << unigrams.size() <<"\n";
      unigrams = Tokenize::toLower(unigrams);
      //      std::cout << "lowerized "<< unigrams.size() << "\n";
      unigrams = Tokenize::filterStopWords(unigrams, stopSet);
      //      std::cout << "filtered "<< unigrams.size() << "\n";  

      for(std::vector<std::string>::iterator uniIt = unigrams.begin(); uniIt != unigrams.end(); uniIt++) {
        const unsigned char* unigram = (const unsigned char*)((*uniIt).c_str());
	//        std::cout << "unigram : "<< *uniIt <<"\n";
        librdf_node* uniLiteral  = librdf_new_node_from_literal(rdfparser.getWorld(), unigram, NULL,0);
        librdf_node* currentSub = librdf_new_node_from_node(subject);
        librdf_node* currentPred = librdf_new_node_from_node(uniPredNode);
        librdf_statement* toadd = librdf_new_statement_from_nodes(rdfparser.getWorld(), currentSub, currentPred, uniLiteral); // nodes get owned by the statemnt, so we should not free it
        librdf_model_add_statement(rdfparser.getModel(), toadd);

        librdf_free_statement(toadd);
      }
      
      std::vector<std::string> bigrams = Tokenize::ngrams(unigrams,2);
      for(std::vector<std::string>::iterator bgIt = bigrams.begin(); bgIt != bigrams.end(); bgIt++) {
        //std::cout << "bigram : "<< *bgIt <<"\n"; 
        const unsigned char* bigram = (const unsigned char*)((*bgIt).c_str()); 
        librdf_node* biLiteral  = librdf_new_node_from_literal(rdfparser.getWorld(), bigram, NULL,0);
        librdf_node* currentSub = librdf_new_node_from_node(subject);
        librdf_node* currentPred = librdf_new_node_from_node(biPredNode);
        librdf_statement* toadd = librdf_new_statement_from_nodes(rdfparser.getWorld(), currentSub, currentPred, biLiteral);
        librdf_model_add_statement(rdfparser.getModel(),  toadd);

        librdf_free_statement(toadd);
      }
      
      //  std::cout << "size of unigram : "<<unigrams.size(); 
      std::vector<std::string> trigrams = Tokenize::ngrams(unigrams,3);
      for(std::vector<std::string>::iterator triIt = trigrams.begin(); triIt != trigrams.end(); triIt++) {
	//  std::cout << "trigram : "<< *triIt <<"\n";
        const unsigned char* trigram = (const unsigned char*)((*triIt).c_str());
        librdf_node* triLiteral  = librdf_new_node_from_literal(rdfparser.getWorld(), trigram, NULL,0);
        librdf_node* currentSub = librdf_new_node_from_node(subject);
        librdf_node* currentPred = librdf_new_node_from_node(triPredNode); 
        librdf_statement* toadd = librdf_new_statement_from_nodes(rdfparser.getWorld(), currentSub, currentPred, triLiteral);
        librdf_model_add_statement(rdfparser.getModel(), toadd);

        librdf_free_statement(toadd);
      } 
      
    }
    librdf_stream_next(stream);
  }

  librdf_free_uri(uniPredUri);
  librdf_free_uri(biPredUri);
  librdf_free_uri(triPredUri);
  librdf_free_node(uniPredNode);
  librdf_free_node(biPredNode);
  librdf_free_node(triPredNode);
}

int main(int argc, char* argv[]) {
  bool newRepository = false;
  cmdArg::options_description cmdDesc("Allowed command line options");

  cmdDesc.add_options()
    ("help","the help message")
    ("parse,P",cmdArg::value< std::vector<std::string> >(),"the file or base dir to parse")
    ("stop-file,SF",cmdArg::value<std::string>(),"the file containing the stop words")
    ("query,Q",cmdArg::value<std::string>(),"query file or string to process")
    ("repo,R",cmdArg::value<std::string>(),"Repository to query/store data")
    ("repo-name,RN",cmdArg::value<std::string>(),"Name of the repository..the files in repo will be created with this name")
    ("new-repo,NR",cmdArg::value<bool>(&newRepository)->default_value(false),"true/false option to create/use-existing repository");  
  
  cmdArg::variables_map cmdMap;
  cmdArg::store(cmdArg::parse_command_line(argc, argv,cmdDesc), cmdMap);
  cmdArg::notify(cmdMap);  
   
  RDFParser rdfparser{};
  bool newRepo = false;
  std::string dirToStore;
  std::vector<std::string> parsePaths;
  std::string repoName;
  std::string stopFile;
  
  if(cmdMap.count("new-repo")) {
    newRepo = cmdMap["new-repo"].as<bool>();
  }  
  if(cmdMap.count("parse")) {
    parsePaths = cmdMap["parse"].as< std::vector<std::string> >();   // You need to remove the last character / from path
    
  }
  
  if(cmdMap.count("stop-file")) {
    stopFile = cmdMap["stop-file"].as<std::string>();
  }
  
  if(cmdMap.count("repo") && cmdMap.count("repo-name")) {
    dirToStore = cmdMap["repo"].as<std::string>();
    repoName = cmdMap["repo-name"].as<std::string>();;
    rdfparser.initRDFParser(repoName, dirToStore, newRepo);
  }
  else {
    std::cout << "repository path or repository name not specified, use --repo-name and --repo\n";
  }
 

  for (std::vector<std::string>::iterator pathIt = parsePaths.begin(); pathIt != parsePaths.end(); pathIt++) {
    std::string path = *pathIt;
    struct dirent *dirStruct;
    DIR *directory =  opendir(path.c_str());
  
    if(directory != NULL) {
      while((dirStruct = readdir(directory)) != NULL) {
        std::string fileName(dirStruct->d_name);
        if(fileName.compare(".") ==0  || fileName.compare("..") == 0 || fileName.rfind(".nt") == std::string::npos)
          continue;   
        std::string fullName = "file://"+path+"/"+fileName;
      
	rdfparser.parse(fullName);
	//    generateAliases(rdfparser, path+"/"+fileName, stopFile);
        std::cout << "parsed file : "+fullName + "\n" ; 
      }
    }
    else {
      FILE* infilePtr = fopen(path.c_str(), "r");
      if (infilePtr != NULL)  {
        fclose(infilePtr);
        std::string inFile;  
        if (path.rfind(".nt") != std::string::npos) {
          inFile = "file://" + path;
	  rdfparser.parse(inFile); 
	  //          generateAliases(rdfparser, path, stopFile);
	  std::cout << "parsed file :"+ inFile + "\n";
        }
      }
      else {
        std::cout << "cannot open input file : " << path.c_str();
      }
    }
  } 
}

