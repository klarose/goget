#ifndef CMD_LINE_ARGS_H
#define CMD_LINE_ARGS_H
#include <string>
#include <cstdint>

//! Simple structure to contain command line arguments for the program.
struct CmdLineArgs
{
    //! The file we're going to fetch
    std::string url;

    //! The host to get the file from
    std::string hostName;

    //! The port at which the http server lives
    std::string port;

    //! The path on the host to get
    std::string pathToFetch;

    //! The to which we'll write
    std::string outFile;
};
#endif // CMD_LINE_ARGS_H
