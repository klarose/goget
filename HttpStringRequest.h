#ifndef HTTP_STRING_REQUEST_H
#define HTTP_STRING_REQUEST_H

#include <string>
#include <vector>

//! A simple class to build an http get request and return it as a list of
//! strings. Could be extended in the future to support more than just GET.
class HttpStringRequest
{
public:

    //! Build a basic get request for @c path
    //! @param path The path to get
    HttpStringRequest(const std::string& path);
    
    //! Adds a header to the request
    //! @param header The header to add. Must not contain any special characters such
    //!               as newlines.
    void AddHeader(const std::string& header);

    const std::vector<std::string>& GetRequest() const;

private:

    //! The line ending for each line in an http request
    static const std::string cs_lineEnding;
    
    //! The version of http for this get request
    static const std::string cs_http_version_string;

    //! The series of lines forming the request, including line endings.
    std::vector<std::string> m_requestStrings;
};
#endif // HTTP_STRING_REQUEST_H
