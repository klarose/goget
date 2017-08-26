#include "CmdLineArgs.h"
#include <stdio.h>

#include <boost/program_options.hpp>
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

int main(int argc, char** argv)
{
    CmdLineArgs args = GetArgsFromCmdLine(argc, argv);

    printf("Getting '%s', writing to '%s'\n",
            args.url.c_str(),
            args.outFile.c_str());

    return 0;
}
