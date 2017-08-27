#ifndef RESPONSE_STREAM_H
#define RESPONSE_STREAM_H

#include <string>

//! Asynchronously streams data from elsewhere.
class AsyncResponseStream
{
public:

    AsyncResponseStream() = default;
    
    //! Note: data may be binary. std::string handles it nicely if constructed
    //! correctly.
    void ProduceData(const std::string& data);

    std::pair<bool, std::string> ReadUntil(const std::string& toMatch);

    std::string ReadRawData();

private:
    std::string m_streamBuffer;
};
#endif // RESPONSE_STREAM_H
