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
    
    std::vector<int> fds;
    std::vector<HttpStringRequest> requests;
    std::vector<AsyncResponseStream> responseStreams;
    std::vector<HttpResponseStreamer> httpResponses;

    for(int i = 0; i < 4; ++i)
    {
        int fd = ConnectToURI(args);
        if(fd < 0)
        {
            printf("Failed to connect\n");
            return -1;
        }
        
        uint64_t start = i * 1024 * 1024;
        uint64_t end = ((i+1) * 1024 * 1024) - 1;
        fds.push_back(fd);
        HttpStringRequest request(args.pathToFetch);
        request.AddHeader("Host: " + args.hostName);
        request.AddHeader("Range: bytes=" + std::to_string(start) + "-" + std::to_string(end));
        requests.push_back(request); 
        responseStreams.push_back(AsyncResponseStream());
    }
    
    for(int i = 0; i < requests.size(); ++i)
    {
        HttpStringRequest &request = requests[i];
        for(const auto& line : request.GetRequest())
        {
            int ret = send(fds[i], line.c_str(), line.length(), 0);
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
        httpResponses.emplace_back(std::move(HttpResponseStreamer(responseStreams[i])));
    }
    
    std::fstream output;

    output.open(args.outFile, std::fstream::out | std::fstream::binary);
    for(int i = 0; i < 4; ++i)
    {
        AsyncResponseStream &stream = responseStreams[i];
        HttpResponseStreamer &response = httpResponses[i];
        char buf[8192];
        int received = 0;
        received = recv(fds[i], &buf, sizeof(buf), 0);
        printf("Received %i\n", received);
        printf("%s", buf);
        stream.ProduceData(std::string(buf, received));

        while(!response.HasError() && !response.HeadersDone())
        {
            response.HandleNewData();
        }

        printf("Length: %u\n", (uint32_t)response.GetContentLength());
        
        output.seekp(i * 1024 * 1024);
        do
        {
            std::string data = response.GetDataChunk();
            output << data;
            if(data.empty())
            {
                received = recv(fds[i], &buf, sizeof(buf), 0);
                printf("Received an extra %i data\n", received);
                stream.ProduceData(std::string(buf, received));
            }

        } while(!response.IsComplete());

    }

    return 0;
}
