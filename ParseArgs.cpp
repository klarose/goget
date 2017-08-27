#include "ParseArgs.h"

#include <boost/program_options.hpp>
#include <boost/network/uri.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace po = boost::program_options;

ParseArgs::ParseArgs(int argc, char** argv):
    m_argumentsValid(false)
{
    if(ParseUrlAndFile(argc, argv))
    {
        m_args.outFile = m_file;
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
        ("out", po::value<std::string>(),  "The file to write");

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

    if(vars.count("out") != 1)
    {
        m_errorMsg = "Exactly one output file must be provided.";
        return false;
    }

    // parse out the url so we can build the argument struct
    m_url = vars["url"].as<std::string>();
    m_file = vars["out"].as<std::string>();

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

    m_args.url = m_url;
    m_args.hostName = toGet.host();
    m_args.pathToFetch = toGet.path();

    // get the optional port
    m_args.port = "80";

    if(!toGet.port().empty())
    {
        m_args.port = toGet.port();
    }

    return true;
}
