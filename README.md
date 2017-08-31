# goget
A simple application to retrieve files over http in chunks.

goget is an application written in C++ which retrieves a file in chunks using parallel tcp connections to an http server.

## Building

Built and tested on Ubuntu 16.10. Should work on most other things -- nothing terribly unportable. 

Testing was performed using gcc 6.2.0.


To build, you may need to install:
 * libboost-program-options-dev
 * libcppnetlib-dev << depends on a bunch of boost stuff

 After grabbing the source, just:
 ```
 make
 ```
 It will create a 'goget' binary in the current directory.
 Note that libboost-program-options1.61.0 seems to generate a warning with -Wall, so don't be alarmed if ParseArgs.cpp warns.


## Running
To run, you may need to install the following runtime libraries:
  * libcppnetlib0
  * libboost-program-options1.61.0
  * libboost-system1.61.0

The application requires two command line arguments:
 * --url _http-url_: The URL to fetch
 * --file _filename_: Where to place the data fetched from _http-url_.
 * --num-chunks _num-chunks_: The number of chunks to download. Each chunk is of size _chunk-size_. Defaults to 4.
 * --chunk-size _chunk-size_: The size of each chunk in bytes. Defaults to 1048576.

The total size off the download will be the minumum of:
 * _num-chunks_ * _chunk-size_ in bytes
 * The size of the file on the server.

## How it works

### Workflow
The application operates in three phases.
 
1. Initialization
 * Parse arguments
 * Create connections to url: one per chunk
 * Send requests: one per chunk
2. Response header parsing
 * Reads headers until they are done
 * Determines the amount of available data from the content-length header.
3. Data download
 * For each chunk, writes whatever is available to the appropriate spot in the output file
 * Repeats until all chunks are complete

### Design



## Limitations
 * The application does not support HTTPS
 * The application does not support redirects, or really anything other than partial content responses
 * The application has limited error handling (e.g. it may hang if a connection is closed prior to the down completing).
 * The application cannot determine the size of the file to be downloaded. It will download as much as you tell it to,
   or as much as exists on the server if you tell it to download more than its size.
 * The application doesn't check whether the file you provided to write to can actually be opened. Make sure it is writeable
    (This is a simple fix if anyone wants to do it. :))
 * The application doesn't deal with running out of disk space.
