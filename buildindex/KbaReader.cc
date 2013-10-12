#include<fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <lzma.h>
#include <errno.h>
#include<iostream>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <thrift/transport/TFDTransport.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <streamcorpus_types.h>
#include <streamcorpus_constants.h>


using namespace streamcorpus;

using namespace boost;

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;


static lzma_stream getStream() {
  lzma_stream strm = LZMA_STREAM_INIT;
  return strm;
}

/**
 * thrift decode logic 
 * first you have to initialze  transport and then protocol.
 */
static void thriftDecode(const char* filename)
{ 
  int fd = open(filename, O_RDONLY);
  shared_ptr<TFDTransport> fdtransport(new TFDTransport(fd));
  shared_ptr<TBufferedTransport> bftransport(new TBufferedTransport(fdtransport));
  shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(bftransport));
  bftransport->open();
  StreamItem streamitem;
  int count =0;
  while(true) {
    try  {
        streamitem.read(protocol.get());
	std::stringbuf doc;
        doc.putn("<DOC>\n",6);
        doc.putn("<DOCNO>\n",8);
        doc.putn(
        doc.putn("</DOCNO>",8); 
        doc.putn("</h</DOC>",6);
	//	std::string content = streamitem.doc_id;
	//std::cout << content;
        count++;
    }
    catch(TTransportException texception)
    { 
      bftransport->flush();
      bftransport->close();
      //      break;
      std::cout <<  count;
      break;
    }
  }  
} 

/**
 * The decompress code copied and modified from tukaani xz utils  doc/examples directory.
 */

static bool decompress(lzma_stream *strm, const char *inname, FILE *infile, FILE *outfile)
{

	lzma_action action = LZMA_RUN;

	uint8_t inbuf[BUFSIZ];
	uint8_t outbuf[BUFSIZ];

	strm->next_in = NULL;
	strm->avail_in = 0;
	strm->next_out = outbuf;


	strm->avail_out = sizeof(outbuf);

	while (true) {
		if (strm->avail_in == 0 && !feof(infile)) {
			strm->next_in = inbuf;
			strm->avail_in = fread(inbuf, 1, sizeof(inbuf),
					infile);

			if (ferror(infile)) {
				fprintf(stderr, "%s: Read error: %s\n",
						inname, strerror(errno));
				return false;
			}

			// Once the end of the input file has been reached,
			// we need to tell lzma_code() that no more input
			// will be coming. As said before, this isn't required
			// if the LZMA_CONATENATED flag isn't used when
			// initializing the decoder.
			if (feof(infile))
				action = LZMA_FINISH;
		}

		lzma_ret ret = lzma_code(strm, action);

		if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t write_size = sizeof(outbuf) - strm->avail_out;

			if (fwrite(outbuf, 1, write_size, outfile)
					!= write_size) {
				fprintf(stderr, "Write error: %s\n",
						strerror(errno));
				return false;
			}

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
				msg = "Compressed file is truncated or "
						"otherwise corrupt";
				break;

			default:
				// This is most likely LZMA_PROG_ERROR.
				msg = "Unknown error, possibly a bug";
				break;
			}

			fprintf(stderr, "%s: Decoder error: "
					"%s (error code %u)\n",
					inname, msg, ret);
			return false;
		}
	}
}

static bool
init_decoder(lzma_stream *strm)
{
	lzma_ret ret = lzma_stream_decoder(
			strm, UINT64_MAX, LZMA_CONCATENATED);

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



