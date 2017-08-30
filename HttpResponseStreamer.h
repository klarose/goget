#ifndef HTTP_RESPONSE_STREAMER_H
#define HTTP_RESPONSE_STREAMER_H
#include "AsyncResponseStream.h"
#include <cinttypes>

//! A class to stream an http response given an async response stream.
//! Currently tailored to the partial content response.
//! Drive this class by telling it to HandleNewData() until indicates
//! that HeadersDone() or that it HasError(). Then, stream data fro
//! GetDataChunk() until IsComplete().
class HttpResponseStreamer
{
public:

    //! Create a response streamer which will stream data from @c stream
    //! when told to
    HttpResponseStreamer(AsyncResponseStream& stream);

    //! Tell the streamer to process data from its input stream.
    //! @return true if the streamer consumed any new data,
    //! false if waiting or more data
    bool HandleNewData();

    //! @return true if an error occured when processing the response
    bool HasError() const;

    //! @return whether all headers have been received
    bool HeadersDone() const;

    //! @return the length, in bytes, of the partial content to be streamed by
    //! GetDataChunk();
    uint64_t GetContentLength() const;

    //! @return whether there is still data to be returned by GetDataChunk();
    bool IsComplete() const;

    //! Streams the most recent received chunk of data to the caller. Note that
    //! calling this will lose any state contained within the returned value,
    //! so the caller is responsible for persisting it if necessary.
    //! @return A binary string containing the most recently received chunk of
    //!         data. Note: this is not null terminated!
    std::string GetDataChunk();

    //! @return the error string or the last encountered error, if any.
    const std::string&  GetError() const;
private:

    //! The states for the partial response parsing.
    enum class ResponseState
    {
        INIT,
        HAVE_VERSION,
        HAVE_CODE,
        HAVE_LENGTH,
        DONE_HEADERS,
        DONE,
        ERROR
    };

    //! the current state of the parser
    ResponseState m_state = ResponseState::INIT;

    //! How much data is in the response. Populated with an actual value when
    //! the content-length header is encountered.
    uint64_t m_contentLength = 0;

    //! How much data has been read so far.
    uint64_t m_dataRead = 0;

    //! The input stream from which the http response is received. 
    AsyncResponseStream& m_stream;

    //! Describes the last error encountered.
    std::string m_error;

    bool HandleInit();
    bool HandleVersion();
    bool HandleCode();
    bool HandleLength();
};
#endif // HTTP_RESPONSE_STREAMER_H
