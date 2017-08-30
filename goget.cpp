#include "ParseArgs.h"
#include <fstream>
#include "ChunkReceiver.h"
#include <memory>


int main(int argc, char** argv)
{
    ParseArgs parser(argc, argv);

    if(!parser.AreArgsValid())
    {
        printf("Error parsing command line: %s\n", parser.GetErrorMessage().c_str());
        printf("%s", parser.GetUsage().c_str());
        return -1;
    }

    const CmdLineArgs& args = parser.GetParsedArgs();

    printf("Getting '%s' from '%s' port '%s', writing to '%s'\n",
            args.pathToFetch.c_str(),
            args.hostName.c_str(),
            args.port.c_str(),
            args.outFile.c_str());
    

    std::fstream output;

    // Break the request up into chunks according to the configuration. 
    // Send the request at the same time.
    std::vector<std::unique_ptr<ChunkReceiver>> receivers;
    for(uint32_t i = 0; i < args.numChunks; ++i)
    {
        receivers.emplace_back(std::unique_ptr<ChunkReceiver>((new ChunkReceiver(args,
                                                                  i * args.chunkSize,
                                                                  output))));
        if(!receivers[i]->ConnectAndSendRequest())
        {
            printf("Failed one of the parallel connections\n");
            return -1;
        }
    }


    // Open the file and stream until all chunks indicate they are complete.
    output.open(args.outFile, std::fstream::out | std::fstream::binary);

    bool allComplete = false;
    while(!allComplete)
    {
        allComplete = true;
        for(auto& receiver : receivers)
        {
            receiver->ProcessResponse();
            allComplete &= receiver->IsDone();
        }
    }
    return 0;
}
