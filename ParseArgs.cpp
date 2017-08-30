#include "ParseArgs.h"

#include <boost/program_options.hpp>
#include <boost/network/uri.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <sstream>

namespace po = boost::program_options;

ParseArgs::ParseArgs(int argc, char** argv):
    m_argumentsValid(false)
{
    if(argc > 0)
    {
        m_programName = argv[0];
    }

    m_args.numChunks = cs_defaultNumChunks;
    m_args.chunkSize = cs_defaultChunkBytes;

    if(ParseUrlAndFile(argc, argv))
    {
        m_argumentsValid = DecomposeUrl();
    }
}

bool ParseArgs::AreArgsValid() const
{
    return m_argumentsValid;
}

const CmdLineArgs& ParseArgs::GetParsedArgs() const
{
    return m_args;
}

const std::string& ParseArgs::GetErrorMessage() const
{
    return m_errorMsg;
}

bool ParseArgs::ParseUrlAndFile(int argc, char** argv)
{
    // start by using boost to parse the arguments.
    po::options_description desc("options");
    desc.add_options()
        ("url", po::value<std::string>(),  "The url to fetch")
        ("chunk-size", po::value<uint64_t>(),  "Size of the chunks to fetch")
        ("num-chunks", po::value<uint64_t>(),  "The number of chunks to fetch")
        ("file", po::value<std::string>(),  "The file to write");

    po::variables_map vars;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vars);
        po::notify(vars); 
    }
    catch(const std::exception& e)
    {
        m_errorMsg = e.what();
        return false;
    }

    // now validate that we have the required arguments.
    if(vars.count("url") != 1)
    {
        m_errorMsg = "Exactly one 'url' must be provided.";
        return false;
    }

    if(vars.count("file") != 1)
    {
        m_errorMsg = "Exactly one output file must be provided.";
        return false;
    }

    if(vars.count("chunk-size") > 0)
    {
        m_args.chunkSize= vars["chunk-size"].as<uint64_t>();
    }

    if(vars.count("num-chunks") > 0)
    {
        m_args.numChunks= vars["num-chunks"].as<uint64_t>();
    }
    // parse out the url so we can build the argument struct
    m_url = vars["url"].as<std::string>();
    m_args.outFile = vars["file"].as<std::string>();

    return true;
}

bool ParseArgs::DecomposeUrl()
{
    const boost::network::uri::uri toGet(m_url);
    if(!toGet.is_valid())
    {
        m_errorMsg = "URL '" + m_url + "' is not valid";
        return false;
    }

    // Default to http, and ensure that if specified, the url is http
    if(!toGet.scheme().empty() && !boost::iequals(toGet.scheme(), "http"))
    {
        m_errorMsg = "Only http is supported";
        return false;
    }

    m_args.hostName = toGet.host();

    // get the optional port
    m_args.port = "80";

    if(!toGet.port().empty())
    {
        m_args.port = toGet.port();
    }
    
    // get the optional path
    m_args.pathToFetch = "/";

    if(!toGet.path().empty())
    {
        m_args.pathToFetch = toGet.path();
    }

    return true;
}

std::string ParseArgs::GetUsage() const
{
    std::ostringstream usage;
    usage << "Usage: " << m_programName << " --url <url_to_fetch> --file <output_file> [options]\n";
    usage << "Options:\n";
    usage << "\t--url\tThe url to go and get. Required.\n";
    usage << "\t--file\tThe file to output to. Required.\n";
    usage << "\t--num-chunks\tThe number of chunks of data to fetch. Default "
          << cs_defaultNumChunks << ".\n";
    usage << "\t--chunks-size\tThe size of each chunk in bytes. Default " 
          << cs_defaultChunkBytes << ".\n";

    return usage.str();
}
