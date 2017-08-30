#ifndef CHUNK_RECEIVER_H
#define CHUNK_RECEIVER_H
#include "CmdLineArgs.h"
#include "AsyncResponseStream.h"
#include "HttpResponseStreamer.h"
#include "HttpStringRequest.h"

#include <ostream>

//! @class ChunkReceiver Receives a chunk of data from a uri using http range
//! requests.
//! Only receives a certain amount at a time so that others may receive in
//! parallel.
//! Response data is written to an output stream in order starting at the
//! offset provided in the constructor.
class ChunkReceiver
{
public:
    //! Construct a chunk receiver to receive traffic to @co outputStream
    //! starting from @c offset in the stream. Assumes that you can seek
    //! randomly in the stream.
    ChunkReceiver(const CmdLineArgs& args,
                  uint64_t offset,
                  std::ostream& outputStream);

    ~ChunkReceiver();
    
    //! Attempt to open a connection to the provided URI, then iniate the
    //! range request.
    //! @return true if the connection attempt was succeful, false otherwise.
    bool ConnectAndSendRequest();

    //! Perform one iteration of received data processing. May block waiting
    //! for data, but will return once some data has been processed.
    void ProcessResponse();

    //! @return true if the request has been fully processed
    bool IsDone() const;

private:

    //! The arguments provided by the user
    const CmdLineArgs m_args;

    //! The offset within the output stream at which we're currently receiving
    //! data
    uint64_t m_fileOffset;

    //! The stream to which we're writing data.
    std::ostream& m_outputStream;

    //! The input stream we're using to stream the response from the tcp socket.
    AsyncResponseStream m_stream;

    //! Used to parse and process the async stream..
    HttpResponseStreamer m_httpResponse;

    //! The fd on which we're performing the http transaction. Will be < 0 if
    //! it is not open.
    int m_socketFd;

    //! Connect to the URI provided.
    //! @return false if the connection failed.
    bool ConnectToURI();

    //! Temporarily holds the input data received from the socket.
    std::vector<char> m_receiveBuffer;

    //! Flag to allow us to control logging the first error we encounter. Set
    //! to true if the error has been logged.
    bool m_loggedError;

    //! Whether we've encountered an error processing the response
    bool m_hadError;
};
#endif // CHUNK_RECEIVER_H
