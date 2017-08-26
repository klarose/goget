#include "CmdLineArgs.h"
#include <stdio.h>

CmdLineArgs GetArgsFromCmdLine(int argc, char** argv)
{
    CmdLineArgs args;
    args.url = "Hello World";
    return args;
}

int main(int argc, char** argv)
{
    CmdLineArgs args = GetArgsFromCmdLine(argc, argv);

    printf("Getting %s\n", args.url.c_str());

    return 0;
}
