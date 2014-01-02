/**
 *@author : ashwani
*/

#ifndef THRIFTDOCUMENTEXTRACTOR_HPP
#define THRIFTDOCUMENTEXTRACTOR_HPP
#include <stdio.h>
#include <iostream>
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
    extern const int BUFFSIZE;
    struct DynBuffer {
      uint8_t* _data;
      int _size; // The number bytes actually stored
      int _capacity; // Total number of bytes available for storage;
      void pushData(uint8_t* load, int loadSize); 
      void resetData();
      DynBuffer() : _data(new uint8_t[BUFFSIZE]), _size(0), _capacity(BUFFSIZE)  {memset(_data, 0, BUFFSIZE);};
      ~DynBuffer() {delete _data;}
    };

    class ThriftDocumentExtractor {
    private:
      std::string _filename;
      //     std::vector<unsigned char> _thriftContent;
      kba::thrift::DynBuffer* _dynBuff;
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
      void reset();      
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

inline void kba::thrift::DynBuffer::resetData() {
  memset(_data, 0, _size);
  _size  = 0;
}

inline void kba::thrift::DynBuffer::pushData (uint8_t* load, int loadSize) {
  if ((_size + loadSize ) > _capacity) {
    _capacity = _size + loadSize + 1;
    uint8_t* relocation = new uint8_t[_capacity];
    memset(relocation, 0,  _capacity );
    memcpy(relocation, _data, _size);
    delete _data;
    _data = relocation;
  }
  memcpy(_data+_size, load, loadSize);
  _size += loadSize;
}
#endif
