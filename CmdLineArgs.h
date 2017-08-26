#ifndef CMD_LINE_ARGS_H
#define CMD_LINE_ARGS_H
#include <string>
#include <cstdint>

//! Simple structure to contain command line arguments for the program.
struct CmdLineArgs
{
    //! The file we're going to fetch
    std::string url;
};
#endif // CMD_LINE_ARGS_H
