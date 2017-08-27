#ifndef CHUNK_RECEIVER_H
#define CHUNK_RECEIVER_H
#include "CmdLineArgs.h"
#include "AsyncResponseStream.h"
#include "HttpResponseStreamer.h"
#include "HttpStringRequest.h"

#include <ostream>

//! Receives a chunk of data from a uri
//! Only receives a certain amount at a time so that others
//! may receive in parallel.
class ChunkReceiver
{
public:
    ChunkReceiver(const CmdLineArgs& args,
                  uint64_t offset,
                  std::ostream& outputStream);

    ~ChunkReceiver();
    
    bool ConnectAndSendRequest();

    void ProcessResponse();
    bool ErrorProcessing();
    bool IsDone() const;

private:
    const CmdLineArgs m_args;

    uint64_t m_fileOffset;
    std::ostream& m_outputStream;
    AsyncResponseStream m_stream;
    HttpResponseStreamer m_httpResponse;
    bool m_error;
    int m_socketFd;

    bool ConnectToURI();

    std::vector<char> m_receiveBuffer;

    bool m_loggedError;
};
#endif // CHUNK_RECEIVER_H
