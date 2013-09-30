/**
 *@author : ashwani
*/

#ifndef THRIFTDOCUMENTEXTRACTOR_HPP
#define THRIFTDOCUMENTEXTRACTOR_HPP
#include <stdio.h>
#include <stdexcept>
#include <string>
#include "lzma.h"
#include <boost/shared_ptr.hpp>
#include "thrift/transport/TBufferTransports.h"
#include "thrift/protocol/TBinaryProtocol.h"

#include "streamcorpus_types.h"
#include "streamcorpus_constants.h"

using namespace streamcorpus;

#include <fstream>

namespace kba
{
  namespace thrift
  {
    
    class ThriftDocumentExtractor {
    private:
      std::string _filename;
      std::vector<uint8_t> _thriftContent;
      lzma_stream _lzmaStream;
      boost::shared_ptr<apache::thrift::transport::TMemoryBuffer>  _memoryTransport;
      boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> _protocol;
      streamcorpus::StreamItem _streamItem;
      
      std::FILE  *_file;

    public:
      void open( const std::string& filename );
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
      
      ThriftDocumentExtractor();
      /**
       * the file must be in xz format.
       * If file not present or xz corrupted then this method construct does not initializes 
       * members of this class and fail silently
       */
      ThriftDocumentExtractor(std::string fileName);
      ~ThriftDocumentExtractor();
    };
  }
}

#endif
