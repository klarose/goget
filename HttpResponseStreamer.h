#ifndef HTTP_RESPONSE_STREAMER_H
#define HTTP_RESPONSE_STREAMER_H
#include "AsyncResponseStream.h"
#include <cinttypes>

//! A class to stream an http response given an async response stream.
//! Currently tailored to the partial content response.
class HttpResponseStreamer
{
public:
    HttpResponseStreamer(AsyncResponseStream& stream);

    //! @return true if the streamer consumed any new data,
    //! false if waiting or more data
    bool HandleNewData();

    bool HasError() const;

    bool HeadersDone() const;
    uint64_t GetContentLength() const;

    bool IsComplete() const;

    std::string GetDataChunk();

    const std::string&  GetError() const;
private:

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

    ResponseState m_state = ResponseState::INIT;
    uint64_t m_contentLength = 0;
    uint64_t m_dataRead = 0;

    AsyncResponseStream& m_stream;

    std::string m_error;

    bool HandleInit();
    bool HandleVersion();
    bool HandleCode();
    bool HandleLength();
};
#endif // HTTP_RESPONSE_STREAMER_H
