#include "ChunkReceiver.h"

#include <stdio.h>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

ChunkReceiver::ChunkReceiver(const CmdLineArgs& args,
                             uint64_t offset,
                             std::ostream& outputStream):
    m_args(args),
    m_fileOffset(offset),
    m_outputStream(outputStream),
    m_httpResponse(m_stream),
    m_socketFd(-1),
    m_receiveBuffer(16384),
    m_loggedError(false),
    m_hadError(false)
{
}

ChunkReceiver::~ChunkReceiver()
{
    if(m_socketFd >= 0)
    {
        close(m_socketFd);
    }
}

bool ChunkReceiver::ConnectAndSendRequest()
{
    if(!ConnectToURI())
    {
        return false;
    }

    // build the range request given the input parameters then send it.
    uint64_t start = m_fileOffset;
    uint64_t end = m_fileOffset + m_args.chunkSize - 1;

    HttpStringRequest request(m_args.pathToFetch);
    request.AddHeader("Host: " + m_args.hostName);
    request.AddHeader("Range: bytes=" + std::to_string(start) + "-" + std::to_string(end));

    for(const auto& line : request.GetRequest())
    {
        int ret = send(m_socketFd, line.c_str(), line.length(), 0);
        if(ret >= 0 && static_cast<size_t>(ret) < line.length())
        {
            printf("Failed to send all the bytes: %i", ret);
            return false;
        }
        else if(ret < 0)
        {
            perror("send");
            return false;
        }
    }

    return true;
}

void ChunkReceiver::ProcessResponse()
{
    // don't do anything if we're done
    if(IsDone())
    {
        return;
    }

    // receive some data
    int received = 0;
    received = recv(m_socketFd, &m_receiveBuffer[0], m_receiveBuffer.size(), 0);

    if(received < 0)
    {
        perror("recv");
        m_hadError = true;
        return;
    }

    // process the data we've received. If the response stream is finished with
    // its headers, move on to the data streaming.

    m_stream.ProduceData(std::string(&m_receiveBuffer[0], received));

    while(!m_httpResponse.HasError() && !m_httpResponse.HeadersDone())
    {
        if(!m_httpResponse.HandleNewData())
        {
            // need more data. Return control to scheduler
            return;
        }
    }

    if(m_httpResponse.HasError() && !m_loggedError)
    {
        printf("Error with response for chunk starting at %" PRIu64 " %s\n", m_fileOffset, m_httpResponse.GetError().c_str());
        m_loggedError = true;
        return;
    }

    std::string data = m_httpResponse.GetDataChunk();
    if(!data.empty())
    {
        m_outputStream.seekp(m_fileOffset);
        m_outputStream << data;
        m_fileOffset += data.size();
    }
}

bool ChunkReceiver::IsDone() const
{
    return m_hadError || m_httpResponse.HasError() || m_httpResponse.IsComplete();
}

bool ChunkReceiver::ConnectToURI()
{
    // use getaddrinfo to get the necessary information for our connect call, resolve
    // the host name, etc.
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Streaming */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	struct addrinfo *result = nullptr;
	int ret = getaddrinfo(m_args.hostName.c_str(), m_args.port.c_str(), &hints, &result);

	if (ret != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return false;
	}

    int socketFd = socket(result->ai_family,
                          result->ai_socktype,
                          result->ai_protocol);

    if(socketFd < 0)
    {
        perror("socket");
        return false;
    }
    
    int connectResult = connect(socketFd,
                                result->ai_addr,
                                result->ai_addrlen);
    if(connectResult != 0)
    {
        perror("connect");
        return false;
    }

    m_socketFd = socketFd;

    return true;
}

