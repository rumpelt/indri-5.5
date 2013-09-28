#include "indri/ThriftDocumentExtractor.hpp"

lzma_stream indri::parse::ThriftDocumentExtractor::_lzmaStream = LZMA_STREAM_INIT;

void indri::parse::ThriftDocumentExtractor::open( const std::string& filename ) {

  _filename = filename;
  _file = std::fopen(filename.c_str(), "rb");

  // _file = boost::shared_ptr<FILE>(filePtr);

  if(_file == NULL) {
    // LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );
  }
  
  indri::parse::ThriftDocumentExtractor::init_decoder(&indri::parse::ThriftDocumentExtractor::_lzmaStream);
  //cout << "\ndecompressing \n";
  
  if(!indri::parse::ThriftDocumentExtractor::decompress(&indri::parse::ThriftDocumentExtractor::_lzmaStream)) {
    std::cout << "Could not decmpress xz file :"+filename+"\n";
    return;
  }
  lzma_end(&indri::parse::ThriftDocumentExtractor::_lzmaStream);

  _memoryTransport = boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>(new apache::thrift::transport::TMemoryBuffer(_thriftContent.data(), _thriftContent.size()));

  _protocol = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(_memoryTransport));

}

bool indri::parse::ThriftDocumentExtractor::init_decoder(lzma_stream *strm) {
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

bool indri::parse::ThriftDocumentExtractor::decompress(lzma_stream *strm){
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
      for(int idx = 0 ; idx < write_size; idx++)
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
      cout << msg;
      return false;
    }
  }
}


Sentence* indri::parse::ThriftDocumentExtractor::getSentence(ContentItem& contentItem, int sentenceId, std::string taggerId="lingpipe")  {
  std::vector<Sentence> stmtList;
  if(contentItem.sentences.find(taggerId) != contentItem.sentences.end())
    stmtList = contentItem.sentences[taggerId];
  if (stmtList.size() >= sentenceId)
    return &stmtList[sentenceId];
  else
    return NULL;
}

void indri::parse::ThriftDocumentExtractor::iterateOverSentence(StreamItem& streamItem, std::string& taggerId) {
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
void indri::parse::ThriftDocumentExtractor::iterateOverRelations(StreamItem& streamItem) {
  ContentItem body = streamItem.body;
  std::vector<Relation> relations;
  

  if (body.relations.find("lingpipe") != body.relations.end())
    relations = body.relations["lingpipe"];
  else if(body.relations.find("stanford") != body.relations.end())
   relations = body.relations["stanford"];
  for (std::vector<Relation>::iterator riter = relations.begin();   riter != relations.end(); ++riter) {
    Relation relation = *riter;
    Sentence* subject = indri::parse::ThriftDocumentExtractor::getSentence(body, relation.sentence_id_1);
    std::vector<Token> subTokens  = indri::parse::ThriftDocumentExtractor::getMentionedTokens(subject, relation.mention_id_1);

    Sentence* predicate = indri::parse::ThriftDocumentExtractor::getSentence(body, relation.sentence_id_2);
    std::vector<Token> predTokens  = indri::parse::ThriftDocumentExtractor::getMentionedTokens(predicate, (MentionID)relation.mention_id_2);
    std::string relationName = relation.relation_name;
    std::cout << subTokens[0].lemma <<" " << relationName << " "<< predTokens[0].lemma;
  }
}


std::vector<Token> indri::parse::ThriftDocumentExtractor::getMentionedTokens(Sentence* sentence, MentionID mentionId) {
  std::vector<Token> tokens;
  for(std::vector<Token>::iterator viter = sentence->tokens.begin(); viter != sentence->tokens.end(); ++viter) {
    Token token = *viter;
    if (token.mention_id == mentionId)
      tokens.push_back(*viter);
  }
  return tokens;
}

StreamItem* indri::parse::ThriftDocumentExtractor::nextStreamItem() {
  if(_memoryTransport == NULL || _protocol == NULL)
    return NULL;
  try {
    _streamItem.read(_protocol.get());
    return &_streamItem;
  }
  catch(apache::thrift::transport::TTransportException texception) {
    return NULL;
  }
}



std::string indri::parse::ThriftDocumentExtractor::getTitle(StreamItem& streamItem) {
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

std::string indri::parse::ThriftDocumentExtractor::getAnchor(StreamItem& streamItem) {
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

indri::parse::UnparsedDocument* indri::parse::ThriftDocumentExtractor::nextDocument() {
  //cout << "Inside next doc";
  if(_memoryTransport == NULL || _protocol == NULL)
    return 0; 
  _document.text = 0;
  _document.textLength = 0;
  _document.content = 0;
  _document.contentLength = 0;
  _document.metadata.clear();

  try {
  
    _streamItem.read(_protocol.get());
    
    std::stringstream doc;
    doc << "<DOC>\n";
    doc << "<DOCNO>\n";
    doc << _streamItem.stream_id.c_str();
    doc << "</DOCNO>\n";
  
    doc  << "<DOCHDR>\n";
    if (_streamItem.abs_url.size() > 0)
      doc << _streamItem.abs_url.c_str();
    doc << "</DOCHDR>\n";
    
    doc << "<TIMESTAMP>\n";
    streamcorpus::StreamTime stime = _streamItem.stream_time;
    doc << fixed << stime.epoch_ticks;
    doc << "</TIMESTAMP>\n";
    

    doc  << "<SOURCE>\n";
    if (_streamItem.source.size() > 0)
      doc << _streamItem.source.c_str();
    doc << "</SOURCE>\n";
    

    doc  << "<MEDIATYPE>\n";
    if (_streamItem.body.media_type.size() > 0)
      doc << _streamItem.body.media_type.c_str();
    doc << "</MEDIATYPE>\n";

    if (_streamItem.other_content.find("title") != _streamItem.other_content.end())
    {
      doc << "<TITLE>\n";
      doc << _streamItem.other_content["title"].raw.data();
      doc << "</TITLE>\n";
    }

    if (_streamItem.other_content.find("anchor") != _streamItem.other_content.end())
    {
      doc << "<ANCHOR>\n";
      doc << _streamItem.other_content["anchor"].raw.data();
      doc << "</ANCHOR>\n";
    }


    doc << "<html>\n";

    if(_streamItem.body.clean_html.size() > 0)
      doc << _streamItem.body.clean_html.data();
    else if(_streamItem.body.raw.size() > 0)
      doc << _streamItem.body.raw.data();
    
    doc << "</html>\n";

    doc << "</DOC>";    
    
    indri::parse::MetadataPair pair;

    pair.value = _filename.c_str();
    pair.valueLength = _filename.length()+1;
    pair.key = "path";
    _document.metadata.push_back( pair );

    _docnostring.assign(_streamItem.stream_id.c_str());
    cleanDocno();
    pair.value = _streamItem.stream_id.c_str();
    pair.valueLength = _docnostring.length()+1;
    pair.key = "docno";
    _document.metadata.push_back( pair );

    
    std::string docContent = doc.str();
    size_t numChars = docContent.size();
    
    const char* doctext = docContent.c_str();
    _document.text = doctext;
    _document.textLength = numChars + 1; //for null character
    _document.content = doctext;
    _document.contentLength = numChars; // no null
    
    return &_document;
  } catch(apache::thrift::transport::TTransportException texception) {
    return 0;
  }
}

void indri::parse::ThriftDocumentExtractor::close() {
  fclose(_file);
//  int a;
}

indri::parse::ThriftDocumentExtractor::~ThriftDocumentExtractor()
{
  _thriftContent.clear();
  indri::parse::ThriftDocumentExtractor::close();
}
