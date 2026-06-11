#include "maifetch/http.hpp"

#include <curl/curl.h>

#include <memory>
#include <stdexcept>

namespace maifetch {
namespace {

size_t write_body(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* output = static_cast<std::vector<unsigned char>*>(userdata);
    const auto bytes = size * nmemb;
    output->insert(output->end(), reinterpret_cast<unsigned char*>(ptr), reinterpret_cast<unsigned char*>(ptr) + bytes);
    return bytes;
}

} // namespace

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

HttpResponse HttpClient::get(std::string_view url, const std::map<std::string, std::string>& headers) const {
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) throw std::runtime_error("could not initialize curl");

    curl_slist* raw_headers = nullptr;
    for (const auto& [key, value] : headers) {
        raw_headers = curl_slist_append(raw_headers, (key + ": " + value).c_str());
    }
    std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> header_guard(raw_headers, curl_slist_free_all);

    HttpResponse response;
    curl_easy_setopt(curl.get(), CURLOPT_URL, std::string(url).c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, raw_headers);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 30000L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 30000L);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_body);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.bytes);

    const auto code = curl_easy_perform(curl.get());
    if (code != CURLE_OK) throw std::runtime_error(curl_easy_strerror(code));
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.status);
    response.body.assign(response.bytes.begin(), response.bytes.end());
    return response;
}

} // namespace maifetch
