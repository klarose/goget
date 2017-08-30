#ifndef RESPONSE_STREAM_H
#define RESPONSE_STREAM_H

#include <string>

//! Asynchronously streams data from elsewhere. Use this class by producing
//! data into it from somewhere, then consuming it in two ways:
//! 1) Retrieve all data currently being stored, and empty the internal buffer.
//! 2) Return a subset of the stream, starting from the beginning, only if some
//!    matching characters were found. This is useful for parsing headers/etc
//!    if unsure whether the headers have been received yet.
class AsyncResponseStream
{
public:

    AsyncResponseStream() = default;
    
    //! Note: data may be binary. std::string handles it nicely if constructed
    //! correctly.
    void ProduceData(const std::string& data);

    //! Reads until the provided string has been matched. Returns the string 
    //! up to, but not inclduing, the matching characters. Both are removed
    //! for the internal stream buffer on match. If no match occurs, nothing
    //! is removed. Note that the returned string may be non-null-terminated
    //! binary string.
    //! @return <true, up-to-matched> if @c toMatch was found
    //!         <false, ""> otherwise.
    std::pair<bool, std::string> ReadUntil(const std::string& toMatch);

    //! @return the entirety of the stream buffer, emptying it. May be binary data.
    std::string ReadRawData();

private:

    //! Holds any data we're waiting to process.
    std::string m_streamBuffer;
};
#endif // RESPONSE_STREAM_H
