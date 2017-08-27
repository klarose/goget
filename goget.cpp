#include "ParseArgs.h"
#include "HttpStringRequest.h"
#include "AsyncResponseStream.h"
#include "HttpResponseStreamer.h"
#include <fstream>

#include <stdio.h>
#include <cstring>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int ConnectToURI(const CmdLineArgs& args)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	struct addrinfo *result = nullptr;
	int ret = getaddrinfo(args.hostName.c_str(), args.port.c_str(), &hints, &result);

	if (ret != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
	}

    int socketFd = socket(result->ai_family,
                          result->ai_socktype,
                          result->ai_protocol);

    if(socketFd < 0)
    {
        perror("socket");
        return -1;
    }
    
    int connectResult = connect(socketFd,
                                result->ai_addr,
                                result->ai_addrlen);
    if(connectResult != 0)
    {
        perror("connect");
        return -1;
    }

    return socketFd;
}

int main(int argc, char** argv)
{
    ParseArgs parser(argc, argv);

    if(!parser.AreArgsValid())
    {
        printf("Error parsing command line: %s\n", parser.GetErrorMessage().c_str());
        exit(1);
    }

    const CmdLineArgs& args = parser.GetParsedArgs();

    printf("Url: %s\n", args.url.c_str());

    printf("Getting '%s' from '%s' port '%s', writing to '%s'\n",
            args.pathToFetch.c_str(),
            args.hostName.c_str(),
            args.port.c_str(),
            args.outFile.c_str());

    int fd = ConnectToURI(args);
    if(fd >= 0)
    {
        printf("Successfully connected\n");
    }
    else
    {
        printf("Failed to connect\n");
        return -1;
    }

    HttpStringRequest request(args.pathToFetch);
    request.AddHeader("Host: " + args.hostName);
    request.AddHeader("Range: bytes=0-1023");
    
    for(const auto& line : request.GetRequest())
    {
        int ret = send(fd, line.c_str(), line.length(), 0);
        if(ret >= 0 && static_cast<size_t>(ret) < line.length())
        {
            printf("Failed to send all the bytes: %i", ret);
            return -1;
        }
        else if(ret < 0)
        {
            perror("send");
            return -1;
        }
    }
    
    char buf[8192];
    int received = 0;
    AsyncResponseStream stream;
    HttpResponseStreamer response(stream);
    received = recv(fd, &buf, sizeof(buf), 0);
    stream.ProduceData(std::string(buf, received));
    printf("%s", buf);

    while(!response.HasError() && !response.HeadersDone())
    {
        response.HandleNewData();
    }
    
    std::fstream output;

    output.open(args.outFile, std::fstream::out | std::fstream::binary);
    output.seekp(4096);
    do
    {
        std::string data = response.GetDataChunk();
        output << data;
        if(data.empty())
        {
            received = recv(fd, &buf, sizeof(buf), 0);
            printf("Received an extra %i data\n", received);
            stream.ProduceData(std::string(buf, received));
        }

    } while(!response.IsComplete());

    printf("Length: %u\n", (uint32_t)response.GetContentLength());

    return 0;
}
