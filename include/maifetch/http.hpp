#pragma once

#include <map>
#include <string>
#include <vector>

namespace maifetch {

struct HttpResponse {
    long status = 0;
    std::string body;
    std::vector<unsigned char> bytes;
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    HttpResponse get(std::string_view url, const std::map<std::string, std::string>& headers = {}) const;
};

} // namespace maifetch
