#include "AsyncResponseStream.h"

void AsyncResponseStream::ProduceData(const std::string& data)
{
    m_streamBuffer += data;
}

std::pair<bool, std::string> AsyncResponseStream::ReadUntil(const std::string& toMatch)
{
    // not the most efficient, since we don't track where we last searched,
    // but good enough for now
    std::string::size_type loc = m_streamBuffer.find(toMatch);

    if(loc != std::string::npos)
    {
        std::pair<bool, std::string> toReturn;
        // we don't want the matching characters. Strip them out.
        toReturn = std::make_pair(true, m_streamBuffer.substr(0, loc));

        // we have the location of the first character, which means we want to
        // take that amount + the length of the search variable.
        size_t toRemove = loc + toMatch.length();
        m_streamBuffer.erase(0, toRemove);
        return toReturn;
    }

    return std::make_pair(false, std::string());
}

std::string AsyncResponseStream::ReadRawData()
{
    std::string toReturn;
    toReturn.swap(m_streamBuffer);
    return toReturn;
}
