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
 * --url _http-url_: The URL to fetch. Note: requires 'http://'
 * --file _filename_: Where to place the data fetched from _http-url_.

It has two optional arguments to control the size and parallelism of the download:
 * --num-chunks _num-chunks_: The number of chunks to download. Each chunk is of size _chunk-size_. Defaults to 4.
 * --chunk-size _chunk-size_: The size of each chunk in bytes. Defaults to 1048576.

The total size off the download will be the minumum of:
 * _num-chunks_ * _chunk-size_ in bytes
 * The size of the file on the server.

Example:

Fetch two chunks of 100 bytes each from the root of www.cnn.com, and write them to index.html.
```
./goget --url http://www.cnn.com --file index.html --num-chunks 2 --chunk-size 100
Getting '/' from 'www.cnn.com' port '80', writing to 'index.html'

wc -c index.html 
200 index.html

```

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
#### Approach to Parallell Download
One of the goals of goget is to download the requested file quickly. To do so, goget breaks the request
into multiple chunks. goget streams each chunk over a separate tcp connection, which can increase the throughput under certain circumstances.

goget is single threaded for simplicity. Further, it uses blocking sockets. This does not mean that the chunks are not downloaded in parallel -- indeed, if one socket blocks waiting for data, the other sockets will continue to receive data into the kernel's receive buffer. Once the blocked socket returns and its data is processed, the application moves on to the other sockets, processing their waiting data.

A more effective approach would have been to use select or epoll. However, for simplicity, blocking sockets did the trick. The overall application was designed with an event driven model in mind. A few approaches could achieve this with some minor refactoring:
 * Expose the socket fd from ChunkReceiver -- have the main loop use this for epoll/select
 * Create a new class which owns the connections and hooks them in to select/epoll. When data is available, call back to the associated ChunkReceiver with the data (e.g. have ProcessResponse() take the data as input). This could be further abstracted into a generic callback if desired. Some more thought would need to go in to the send functionality in this case -- how will ChunkReceiver send the request is the socket is owned by something else? We could probably hook that up in a similar fashion -- have epoll let ChunkReceiver know that it can send data, and tell it what was sent.

Each chunk performs its own dns request to get connect to the server. This means that there is a chance that each chunk comes from a different source, particularly if the host is doing dns load balancing. This has some disadvantages and advantages:

Good:
 * Less load is placed on a given host
Bad:
 * If the host isn't serving up consistent data, the file could be corrupted. This shouldn't happen, but who knows with these crazy web 2.0 servers. :)
 * If the DNS server is slow, it could slow down the initial connection.

#### IO

The application streams data as much as possible. This means that if it no longer needs received data, it discards it. This can be either because it has consumed it and gleaned whatever it needs from the data, or because it has already written it to disk. Doing this means that the application likely won't consume much memory, regardless of the size of the file.

Since the application is streaming data, it needs to write to disk whenever it gets data. Since each chunk is streaming independently, this means that the writer will need to seek to the appropriate spot in the file every time it writes -- there is no guarantee that the file offset will be where the writer left it. This has the disadvantage of potentially causing the disk to thrash around a bit. But, it's good enough for this application. Some improvements could be made by increasing the memory use of the application and only flushing to disk when a certain amount of data is ready to be written.

#### Execution
The program starts in goget.cpp, which drives the main execution, as follows.

* It delegates argument parsing to ParseArgs.
* If the arguments are bad, it errors out
* Otherwise, it creates one ChunkReceiver per requested chunk, and sends the request to the host.
* Once the requests have been succesfully sent, it opens the output file, truncating it.
* If one of the requests fails, it errors out.
* It then tells each ChunkReceiver to process the response, which will either process the response
  header as it comes in, or stream data to the file depending on how far along it is.
* Once everything is done, the program exists.

ChunkReceiver delegates much of its responsibility to other classes, which are documented in the source.

## Limitations
 * The application does not support HTTPS
 * The application does not support redirects, or really anything other than partial content responses
 * The application has limited error handling (e.g. it may hang if a connection is closed prior to the download completing).
 * The application cannot determine the size of the file to be downloaded. It will download as much as you tell it to,
   or as much as exists on the server if you tell it to download more than its size.
 * The application doesn't check whether the file you provided to write to can actually be opened. Make sure it is writeable
    (This is a simple fix if anyone wants to do it. :))
 * The application doesn't deal with running out of disk space.
