/**
 *@author : ashwani
*/

#ifndef INDRI_THRIFTDOCUMENTEXTRACTOR_HPP
#define INDRI_THRIFTDOCUMENTEXTRACTOR_HPP

#include <stdio.h>
#include <string>
#include "lzma.h"
#include <boost/shared_ptr.hpp>


#include "indri/UnparsedDocument.hpp"
#include "indri/DocumentIterator.hpp"
#include "lemur/Exception.hpp"

#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif
#include "thrift/transport/TBufferTransports.h"
#include "thrift/protocol/TBinaryProtocol.h"

#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"



#include <fstream>

namespace indri
{
  namespace parse
  {
    
    class ThriftDocumentExtractor : public DocumentIterator {
    private:
      std::string _filename;
      UnparsedDocument _document;
      std::vector<uint8_t> _thriftContent;
      static lzma_stream _lzmaStream;
      boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>  _memoryTransport;
      apache::thrift::protocol::TBinaryProtocol *_protocol;
      streamcorpus::StreamItem _streamItem;
  
      std::FILE  *_file;

    public:
      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
      void close();
      bool init_decoder(lzma_stream *strm);
      bool decompress(lzma_stream *strm);
      ~ThriftDocumentExtractor();
    };
  }
}

#endif //INDRI_THRIFTDOCUMENTEXTRACTOR_HPP
