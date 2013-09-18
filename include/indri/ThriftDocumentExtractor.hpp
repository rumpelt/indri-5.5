/**
 *@author : ashwani
*/

#ifndef INDRI_THRIFTDOCUMENTEXTRACTOR_HPP
#define INDRI_THRIFTDOCUMENTEXTRACTOR_HPP

#include <stdio.h>
#include <stdexcept>
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

using namespace streamcorpus;

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
      boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> _protocol;
      streamcorpus::StreamItem _streamItem;
      
      std::FILE  *_file;

    public:
      void open( const std::string& filename );
      UnparsedDocument* nextDocument();
      std::string getAnchor(StreamItem& streamItem);
      std::string getTitle(StreamItem& streamItem);
      /**
       * dont mix call of nextStreamItem and nextDocument.
       * Only one of them should be called.
       */
      StreamItem* nextStreamItem();
      Sentence* getSentence(ContentItem& contentItem, int  sententceId, std::string taggerId);
  
      std::vector<Token> getMentionedTokens(Sentence* sentence, MentionID mentionid);
      void iterateOverSentence(StreamItem& streamItem, std::string& taggerId);
 
      void iterateOverRelations(StreamItem& streamItem);
      void close();
      bool init_decoder(lzma_stream *strm); // called by open call above
      bool decompress(lzma_stream *strm);
      ~ThriftDocumentExtractor();
    };
  }
}

#endif //INDRI_THRIFTDOCUMENTEXTRACTOR_HPP
