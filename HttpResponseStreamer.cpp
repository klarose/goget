#include "HttpResponseStreamer.h"
#include <stdexcept>

HttpResponseStreamer::HttpResponseStreamer(AsyncResponseStream& stream):
    m_stream(stream)
{
    
}

bool HttpResponseStreamer::HandleNewData()
{
    switch(m_state)
    {
        case ResponseState::INIT:
        {
            return HandleInit();
        }
        case ResponseState::HAVE_VERSION:
        {
            return HandleVersion();
        }
        case ResponseState::HAVE_CODE:
        {
            return HandleCode();
        }
        case ResponseState::HAVE_LENGTH:
        {
            return HandleLength();
        }
        case ResponseState::DONE_HEADERS:
        case ResponseState::DONE:
        case ResponseState::ERROR:
        {
            // nothing to do here
            break;
        }
    }

    return false;
}

bool HttpResponseStreamer::HasError() const
{
    return m_state == ResponseState::ERROR;
}

bool HttpResponseStreamer::HeadersDone() const
{
    return m_state == ResponseState::DONE_HEADERS;
}

uint64_t HttpResponseStreamer::GetContentLength() const
{
    return m_contentLength;
}

bool HttpResponseStreamer::IsComplete() const
{
    return m_state == ResponseState::DONE;
}

bool HttpResponseStreamer::HandleInit()
{
    std::pair<bool, std::string> result = m_stream.ReadUntil(" ");
    if(result.first)
    {
        m_state = ResponseState::HAVE_VERSION;
        return true;
    }

    return false;
}

bool HttpResponseStreamer::HandleVersion()
{
    std::pair<bool, std::string> result = m_stream.ReadUntil(" ");
    if(result.first)
    {
        m_state = ResponseState::HAVE_CODE;
        return true;
    }

    return false;
}

bool HttpResponseStreamer::HandleCode()
{
    // now we're going to read full lines until we get the conent-length
    // header
    std::pair<bool, std::string> result = m_stream.ReadUntil("\r\n");
    if(!result.first)
    {
        return false;
    }

    const std::string& line = result.second;
    const std::string headerSearch = "Content-Length: ";
    size_t loc = line.find(headerSearch);
    if(loc == std::string::npos)
    {
        // not content length. Skip this header.
        return true;
    }

    std::string length = line.substr(headerSearch.length());
    try
    {
        m_contentLength = std::stoul(length);
    }
    catch(const std::invalid_argument& e)
    {
        printf("Content-Length was invalid\n");
        m_state = ResponseState::ERROR;
        return true;
    }

    m_state = ResponseState::HAVE_LENGTH;

    return true;
}

bool HttpResponseStreamer::HandleLength()
{
    //read until the body
    std::pair<bool, std::string> result = m_stream.ReadUntil("\r\n");

    // if we find the end of line, and it's empty, it means we've been signaled
    // that the headers are done
    if(result.first && result.second.empty())
    {
        m_state = ResponseState::DONE_HEADERS;
        return true;
    }

    return false;
}

std::string HttpResponseStreamer::GetDataChunk()
{
    std::string chunk = m_stream.ReadRawData();
    m_dataRead += chunk.size();

    if(m_dataRead >= m_contentLength)
    {
        m_state = ResponseState::DONE;
    }

    return chunk;
}
