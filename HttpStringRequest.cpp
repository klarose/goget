#include "HttpStringRequest.h"
#include <assert.h>

const std::string HttpStringRequest::cs_lineEnding = "\r\n";
const std::string HttpStringRequest::cs_http_version_string = "HTTP/1.1";

HttpStringRequest::HttpStringRequest(const std::string& path)
{
    m_requestStrings.push_back("GET " + path + " " + cs_http_version_string + cs_lineEnding);
    m_requestStrings.push_back(cs_lineEnding);
}

void HttpStringRequest::AddHeader(const std::string& header)
{
    // the construct adds the initial line and a blank line
    assert(m_requestStrings.size() >= 2);

    // replace the old ending with the new header then push the ending empty
    // line back to the end. Simple, allowing us to provide a const GetRequest,
    // while avoiding a function to 'complete' the request.
    m_requestStrings.back() = header + cs_lineEnding;
    m_requestStrings.push_back(cs_lineEnding);
}

const std::vector<std::string>& HttpStringRequest::GetRequest() const
{
    return m_requestStrings;
}
