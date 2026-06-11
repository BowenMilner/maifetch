#include "test.hpp"

#include "maifetch/maitea.hpp"

namespace {

void json_parser_supports_api_shapes() {
    const auto json = maifetch::parse_json(R"({
        "data": [{
            "id": 7,
            "name": {"en": "Song", "jp": ""},
            "artist": {"en": "Artist", "jp": ""},
            "code": "abc"
        }],
        "links": {"first": "/a", "last": "/z", "prev": null, "next": "/b"}
    })");

    const auto track = maifetch::parse_track(json.at("data").at(0));
    const auto links = maifetch::parse_page_links(json.at("links"));
    expect_eq(track.id, 7, "track id should parse");
    expect_eq(track.name.en, "Song", "localized name should parse");
    expect_true(!links.prev.has_value(), "null prev link should parse");
    expect_eq(links.next.value_or(""), "/b", "next link should parse");
}

} // namespace

struct JsonTests {
    JsonTests() {
        json_parser_supports_api_shapes();
    }
} json_tests;

int main() {
    std::cout << "maifetch tests passed\n";
    return 0;
}
