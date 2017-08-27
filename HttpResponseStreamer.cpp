#include "HttpResponseStreamer.h"
#include <stdexcept>

HttpResponseStreamer::HttpResponseStreamer(AsyncResponseStream& stream):
    m_stream(stream)
{
    
}

void HttpResponseStreamer::HandleNewData()
{
    switch(m_state)
    {
        case ResponseState::INIT:
        {
            HandleInit();
            break;
        }
        case ResponseState::HAVE_VERSION:
        {
            HandleVersion();
            break;
        }
        case ResponseState::HAVE_CODE:
        {
            HandleCode();
            break;
        }
        case ResponseState::HAVE_LENGTH:
        {
            HandleLength();
            break;
        }
        case ResponseState::DONE_HEADERS:
        case ResponseState::DONE:
        case ResponseState::ERROR:
        {
            // nothing to do here
            break;
        }
    }
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

void HttpResponseStreamer::HandleInit()
{
    std::pair<bool, std::string> result = m_stream.ReadUntil(" ");
    if(result.first)
    {
        m_state = ResponseState::HAVE_VERSION;
    }
       
}

void HttpResponseStreamer::HandleVersion()
{
    std::pair<bool, std::string> result = m_stream.ReadUntil(" ");
    if(result.first)
    {
        m_state = ResponseState::HAVE_CODE;
    }
}

void HttpResponseStreamer::HandleCode()
{
    // now we're going to read full lines until we get the conent-length
    // header
    std::pair<bool, std::string> result = m_stream.ReadUntil("\r\n");
    if(!result.first)
    {
        return;
    }

    const std::string& line = result.second;
    const std::string headerSearch = "Content-Length: ";
    size_t loc = line.find(headerSearch);
    if(loc == std::string::npos)
    {
        return;
    }

    std::string length = line.substr(headerSearch.length());
    try
    {
        m_contentLength = std::stoul(length);
    }
    catch(const std::invalid_argument& e)
    {
        m_state = ResponseState::ERROR;
        return;
    }

    m_state = ResponseState::HAVE_LENGTH;
}

void HttpResponseStreamer::HandleLength()
{
    //read until the body
    std::pair<bool, std::string> result = m_stream.ReadUntil("\r\n");

    // if we find the end of line, and it's empty, it means we've been signaled
    // that the headers are done
    if(result.first && result.second.empty())
    {
        m_state = ResponseState::DONE_HEADERS;
    }
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
