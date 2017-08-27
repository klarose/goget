#include "ParseArgs.h"

#include <stdio.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


bool ConnectToURI(const CmdLineArgs& args)
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

    return true;
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

    bool connected = ConnectToURI(args);
    if(connected)
    {
        printf("Successfully connected\n");
    }
    else
    {
        printf("Failed to connect\n");
    }

    return 0;
}
