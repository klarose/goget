#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include "CmdLineArgs.h"
#include <string>

//! Parses the arguments necessary to goget from the command line.
class ParseArgs
{
public:

    //! Constructs a command line argument parser for goget.
    //! @param argc The number of arguments to @c argv
    //! @param argv An array of null terminated strings of length @c argc
    ParseArgs(int argc, char** argv);

    //! @return Whether or not the provided arguments were valid.
    bool AreArgsValid() const;

    //! Returns the arguments parsed from the command line. If they were
    //! invalid, returns a default-initialized argument list.
    //! @return The parsed command line arguments/
    const CmdLineArgs& GetParsedArgs() const;

    //! @return A string detailing the first encountered error parsing the
    //!         arguments, if any.
    const std::string& GetErrorMessage() const;

private:

    CmdLineArgs m_args;
    std::string m_errorMsg;
    std::string m_url;
    std::string m_file;
    bool m_argumentsValid;

    //! Parses the url and file from the command line
    //! @param argc the number of entries in @c argv
    //! @param argv the arguments to parse
    //! @return whether parsing was succesful
    bool ParseUrlAndFile(int argc, char** argv);

    //! Extracts the necessary information from the url and places it into
    //! @c m_argumentsValid
    //! @return whether extraction succeeded.
    bool DecomposeUrl();
};
#endif // PARSE_ARGS_H
