#include "CmdLineArgs.h"
#include <stdio.h>

#include <boost/program_options.hpp>
#include <boost/network/uri.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


namespace po = boost::program_options;

CmdLineArgs GetArgsFromCmdLine(int argc, char** argv)
{
    CmdLineArgs args;

    po::options_description desc("options");
    desc.add_options()
        ("url", po::value<std::string>(),  "The url to fetch")
        ("out", po::value<std::string>(),  "The file to write");

    po::variables_map vars;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vars);
        po::notify(vars); 
    }
    catch(const std::exception& e)
    {
        printf("%s\n", e.what());
        exit(1);
    }

    if(vars.count("url") != 1)
    {
        printf("Exactly one 'url' must be provided.\n");
        exit(1);
    }

    if(vars.count("out") != 1)
    {
        printf("Exactly one output file must be provided.\n");
        exit(1);
    }

    args.url = vars["url"].as<std::string>();
    args.outFile = vars["out"].as<std::string>();

    return args;
}

bool ConnectToURI(const boost::network::uri::uri& uri)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	struct addrinfo *result = nullptr;
	int ret = getaddrinfo(uri.host().c_str(), "80", &hints, &result);

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
    CmdLineArgs args = GetArgsFromCmdLine(argc, argv);

    boost::network::uri::uri toGet(args.url);
    if(!toGet.is_valid())
    {
        printf("URL '%s' is not valid.\n", args.url.c_str());
        exit(1);
    }

    printf("Getting '%s' from '%s', writing to '%s'\n",
            toGet.path().c_str(),
            toGet.host().c_str(),
            args.outFile.c_str());

    bool connected = ConnectToURI(toGet);
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
