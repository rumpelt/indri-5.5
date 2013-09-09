#include "indri/ThriftDocumentExtractor.hpp"


void indri::parse::ThriftDocumentExtractor::open( const std::string& filename ) {

  _filename = filename;
  std::FILE *filePtr = std::fopen(filename.c_str(), "rb");

  _file = boost::shared_ptr<FILE>(filePtr);

  if(_file == NULL)
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't open file " + filename + "." );

  lzma_stream strm = LZMA_STREAM_INIT;
  _lzmaStream = boost::shared_ptr<lzma_stream>(&strm);   

  indri::parse::ThriftDocumentExtractor::init_decoder(_lzmaStream);
  cout << "\ndecompressing \n";
  if(!indri::parse::ThriftDocumentExtractor::decompress())
    LEMUR_THROW( LEMUR_IO_ERROR, "Couldn't decompress the xz file " + filename + "." );
  //  cout << _thriftContent.size();
  _memoryTransport= boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>(new apache::thrift::transport::TMemoryBuffer(_thriftContent.data(), _thriftContent.size()));
  _protocol = boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol>(new apache::thrift::protocol::TBinaryProtocol(_memoryTransport));
  cout << "initialze transport layer";
}

bool indri::parse::ThriftDocumentExtractor::init_decoder(boost::shared_ptr<lzma_stream> strm) {
        lzma_ret ret = lzma_stream_decoder(strm.get(), UINT64_MAX, LZMA_CONCATENATED);

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

bool indri::parse::ThriftDocumentExtractor::decompress(){
  lzma_action action = LZMA_RUN;

  uint8_t inbuf[BUFSIZ];
  uint8_t outbuf[BUFSIZ];
  
  _lzmaStream->next_in = NULL;
  _lzmaStream->avail_in = 0;
  _lzmaStream->next_out = outbuf;
  _lzmaStream->avail_out = sizeof(outbuf);

  
  while (true) {
    if (_lzmaStream->avail_in == 0 && !feof(_file.get())) {
      _lzmaStream->next_in = inbuf;
      _lzmaStream->avail_in = fread(inbuf, 1, sizeof(inbuf), _file.get()); 
      if(feof(_file.get()))
        action = LZMA_FINISH;
    }
    lzma_ret ret = lzma_code(_lzmaStream.get(), action);
    if (_lzmaStream->avail_out == 0 || ret == LZMA_STREAM_END) {
      size_t write_size = sizeof(outbuf) - _lzmaStream->avail_out;
      for(int idx = 0 ; idx < write_size; idx++)
        _thriftContent.push_back(outbuf[idx]);
      //      cout <<"\n" +_thriftContent.size();
      _lzmaStream->next_out = outbuf;
      _lzmaStream->avail_out = sizeof(outbuf);    
    }

    if (ret != LZMA_OK) {
      if (ret == LZMA_STREAM_END)
      { 
        cout << "\n returnitng true ";
        return true;
      }
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
indri::parse::UnparsedDocument* indri::parse::ThriftDocumentExtractor::nextDocument() {
  _document.text = 0;
  _document.textLength = 0;
  _document.metadata.clear();

  try {
    _streamItem.read(_protocol.get());
    std::stringbuf doc;
    doc.sputn("<DOC>\n",6);

    doc.sputn("<DOCNO>\n",8);
    doc.sputn(_streamItem.doc_id.c_str(), _streamItem.doc_id.size());
    doc.sputn("</DOCNO>\n",9);
 
    doc.sputn("<DOCHDR>\n",9);
    
    doc.sputn("</DOCHDR>\n",10);
    
    doc.sputn("<html>\n",7);
    doc.sputn(_streamItem.body.raw.data(), _streamItem.body.raw.size());
    doc.sputn("</html>\n",8);

    doc.sputn("</DOC>",6);    
     
    indri::parse::MetadataPair pair;
    pair.value = _filename.c_str();
    pair.valueLength = _filename.length()+1;
    pair.key = "path";
    _document.metadata.push_back( pair );

    _docnostring.assign(_filename.c_str());
    cleanDocno();
    pair.value = _streamItem.doc_id.c_str();
    pair.valueLength = _docnostring.length()+1;
    pair.key = "docno";
    _document.metadata.push_back( pair );

    pair.key = "filetype";
    pair.value = (void*) "TRECWEB";
    pair.valueLength = 8;
    _document.metadata.push_back( pair );
    
    std::string docContent = doc.str();
    size_t numChars = docContent.size();
   
    _document.text = docContent.data();
    _document.textLength = numChars + 1;
    _document.content = docContent.data();
    _document.contentLength = numChars; // no null
    
    return &_document;
  } catch(apache::thrift::transport::TTransportException texception) {
    return 0;
  }
}

void indri::parse::ThriftDocumentExtractor::close() {
  fclose(_file.get());
}
