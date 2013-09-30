#include "ThriftDocumentExtractor.hpp"
#include <iostream>

void kba::thrift::ThriftDocumentExtractor::open( const std::string& filename ) {

  _filename = filename;
  _file = std::fopen(filename.c_str(), "rb");

  
  if(_file == NULL) {
    std::cout << "could not open the thrift file : " << _filename.c_str() << "\n";
  }
  kba::thrift::ThriftDocumentExtractor::_lzmaStream = LZMA_STREAM_INIT;  
  kba::thrift::ThriftDocumentExtractor::init_decoder(&_lzmaStream);
  
  if(!kba::thrift::ThriftDocumentExtractor::decompress(&_lzmaStream)) {
    std::cout << "Could not decmpress xz file :"+filename+"\n";
    return;
  }
  lzma_end(&_lzmaStream);

  _memoryTransport = boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>(new apache::thrift::transport::TMemoryBuffer(_thriftContent.data(), _thriftContent.size()));

  _protocol = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(_memoryTransport));

}

bool kba::thrift::ThriftDocumentExtractor::init_decoder(lzma_stream *strm) {
        lzma_ret ret = lzma_stream_decoder(strm, UINT64_MAX, LZMA_CONCATENATED);

	// Return successfully if the initialization went fine.
	if (ret == LZMA_OK)
		return true;

	const char *msg;
	switch (ret) {
	case LZMA_MEM_ERROR:
		msg = "Memory allocation failed";
		break;

	case LZMA_OPTIONS_ERROR:
		msg = "Unsupported decompressor flags";
		break;

	default:
		msg = "Unknown error, possibly a bug";
		break;
	}

	fprintf(stderr, "Error initializing the decoder: %s (error code %u)\n",
			msg, ret);
	return false;
}

bool kba::thrift::ThriftDocumentExtractor::decompress(lzma_stream *strm){
  lzma_action action = LZMA_RUN;

  uint8_t inbuf[BUFSIZ];
  uint8_t outbuf[BUFSIZ];
  _thriftContent.clear();
  strm->next_in = NULL;
  strm->avail_in = 0;
  strm->next_out = outbuf;
  strm->avail_out = sizeof(outbuf);

  
  while (true) {
    if (strm->avail_in == 0 && !feof(_file)) {
      strm->next_in = inbuf;
      strm->avail_in = fread(inbuf, 1, sizeof(inbuf), _file); 
      if(feof(_file))
        action = LZMA_FINISH;
    }
    lzma_ret ret = lzma_code(strm, action);
    if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
      size_t write_size = sizeof(outbuf) - strm->avail_out;
      //      std::cout << "total ize: " << _thriftContent.max_size() << " : " << _thriftContent.size() << "\n";
      for(size_t idx = 0 ; idx < write_size; idx++) 
        _thriftContent.push_back(outbuf[idx]);

      //      cout <<"\n" +_thriftContent.size();
      strm->next_out = outbuf;
      strm->avail_out = sizeof(outbuf);    
    }

    if (ret != LZMA_OK) {
      if (ret == LZMA_STREAM_END)
        return true;

      const char *msg;
      switch (ret) {
        case LZMA_MEM_ERROR:
	  msg = "Memory allocation failed";
	  break;
	case LZMA_FORMAT_ERROR:
	// .xz magic bytes weren't found.
          msg = "The input is not in the .xz format";
	  break;
        case LZMA_OPTIONS_ERROR:
	  msg = "Unsupported compression options";
	  break;
	case LZMA_DATA_ERROR:
	  msg = "Compressed file is corrupt";
	  break;
	case LZMA_BUF_ERROR:
	  msg = "Compressed file is truncated or otherwise corrupt";       break;
	default:
        // This is most likely LZMA_PROG_ERROR.
	  msg = "Unknown error, possibly a bug";
	  break;
      }
      std::cout << msg;
      return false;
    }
  }
}


Sentence* kba::thrift::ThriftDocumentExtractor::getSentence(ContentItem& contentItem, int sentenceId, std::string taggerId="lingpipe")  {
  std::vector<Sentence> stmtList;
  if(contentItem.sentences.find(taggerId) != contentItem.sentences.end())
    stmtList = contentItem.sentences[taggerId];
  if (stmtList.size() >= sentenceId)
    return &stmtList[sentenceId];
  else
    return NULL;
}

void kba::thrift::ThriftDocumentExtractor::iterateOverSentence(StreamItem& streamItem, std::string& taggerId) {
  ContentItem body = streamItem.body;
  std::vector<Sentence> sentences;
  if (body.sentences.count(taggerId) > 0)
    sentences = body.sentences[taggerId]; 
  for(std::vector<Sentence>::iterator sentit = sentences.begin(); sentit != sentences.end(); sentit++) {
    Sentence  sentence = *sentit;
    for (std::vector<Token>::iterator tokit = sentence.tokens.begin(); tokit != sentence.tokens.end(); tokit++) {
      Token token =  *tokit;
      std::cout << "\nToken: "<< token.token << "\nLemma: " << token.lemma << "\nEntityType: " << token.entity_type << "\nMentionId : " << token.mention_id ;
      std::cout << "\n\n";
    }
    std::cout <<"\n**********Next Sentence*********";
  }
}
void kba::thrift::ThriftDocumentExtractor::iterateOverRelations(StreamItem& streamItem) {
  ContentItem body = streamItem.body;
  std::vector<Relation> relations;
  

  if (body.relations.find("lingpipe") != body.relations.end())
    relations = body.relations["lingpipe"];
  else if(body.relations.find("stanford") != body.relations.end())
   relations = body.relations["stanford"];
  for (std::vector<Relation>::iterator riter = relations.begin();   riter != relations.end(); ++riter) {
    Relation relation = *riter;
    Sentence* subject = kba::thrift::ThriftDocumentExtractor::getSentence(body, relation.sentence_id_1);
    std::vector<Token> subTokens  = kba::thrift::ThriftDocumentExtractor::getMentionedTokens(subject, relation.mention_id_1);

    Sentence* predicate = kba::thrift::ThriftDocumentExtractor::getSentence(body, relation.sentence_id_2);
    std::vector<Token> predTokens  = kba::thrift::ThriftDocumentExtractor::getMentionedTokens(predicate, (MentionID)relation.mention_id_2);
    std::string relationName = relation.relation_name;
    std::cout << subTokens[0].lemma <<" " << relationName << " "<< predTokens[0].lemma;
  }
}


std::vector<Token> kba::thrift::ThriftDocumentExtractor::getMentionedTokens(Sentence* sentence, MentionID mentionId) {
  std::vector<Token> tokens;
  for(std::vector<Token>::iterator viter = sentence->tokens.begin(); viter != sentence->tokens.end(); ++viter) {
    Token token = *viter;
    if (token.mention_id == mentionId)
      tokens.push_back(*viter);
  }
  return tokens;
}

StreamItem* kba::thrift::ThriftDocumentExtractor::nextStreamItem() {
  if(_memoryTransport == NULL || _protocol == NULL)
    return 0;
  try {
    _streamItem.read(_protocol.get());
    return &_streamItem;
  }
  catch(apache::thrift::transport::TTransportException texception) {
    return 0;
  }
}



std::string kba::thrift::ThriftDocumentExtractor::getTitle(StreamItem& streamItem) {
  std::string title;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("title");
    title  = content.raw;
    std::cout << "title raw : " << content.raw << "\n";
    std::cout << "clean visible : " << content.clean_visible << "\n";
    //   std::cout << "#num sentence : "<< content.sentences.at("lingpipe").size(); 
    return title;
  }
  catch(const std::out_of_range& orexpt) {
    return title;
  }
}

std::string kba::thrift::ThriftDocumentExtractor::getAnchor(StreamItem& streamItem) {
  std::string anchor;
  try { 
    ContentItem content;
    content = streamItem.other_content.at("anchor");
    anchor = content.raw;
    std::cout << "anchor raw : " << content.raw << "\n";
    std::cout << "anchor clean visible : " << content.clean_visible << "\n";
    //std::cout << "#num sentence in anchor : "<< content.sentences.at("lingpipe").size() <<"\n"; 
    return anchor;
  }
  catch(const std::out_of_range& orexpt) {
    return anchor;
  }
}

void kba::thrift::ThriftDocumentExtractor::close() {
  if(_file != 0)
    fclose(_file);
}

kba::thrift::ThriftDocumentExtractor::~ThriftDocumentExtractor()
{
  _thriftContent.clear();
  kba::thrift::ThriftDocumentExtractor::close();
}

kba::thrift::ThriftDocumentExtractor::ThriftDocumentExtractor()
{
  _file = 0;
}

/**
 * the file must be in xz format
 */
kba::thrift::ThriftDocumentExtractor::ThriftDocumentExtractor(std::string fileName)
{
  _file = std::fopen(fileName.c_str(), "rb");   
  if (_file != 0) {
    kba::thrift::ThriftDocumentExtractor::_lzmaStream = LZMA_STREAM_INIT;
    kba::thrift::ThriftDocumentExtractor::init_decoder(&_lzmaStream);
  
    if(kba::thrift::ThriftDocumentExtractor::decompress(&_lzmaStream)) {
      lzma_end(&_lzmaStream);
      _memoryTransport = boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>(new apache::thrift::transport::TMemoryBuffer(_thriftContent.data(), _thriftContent.size()));

      _protocol = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(_memoryTransport));
    }
  }
}
