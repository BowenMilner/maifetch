#pragma once

#include "maifetch/http.hpp"
#include "maifetch/tiny_json.hpp"

#include <optional>
#include <string>
#include <vector>

namespace maifetch {

struct Image {
    int id = 0;
    std::string png;
    std::string webp;
};

struct LocalizedName {
    std::string en;
    std::string jp;
};

struct TrackInfo {
    int id = 0;
    std::string code;
    LocalizedName name;
    LocalizedName artist;
};

struct DifficultyLevel {
    int key = 0;
    std::string value;
    std::string label;
};

struct PlayStats {
    int total = 0;
    int wins = 0;
    int vs = 0;
    int sync = 0;
};

struct ProfileOptions {
    Image icon;
    Image icon_deka;
};

struct Profile {
    int id = 0;
    std::string name;
    int rating = 0;
    int rating_highest = 0;
    int level = 0;
    PlayStats play_stats;
    ProfileOptions options;
};

struct Play {
    int id = 0;
    int achievement = 0;
    std::string achievement_formatted;
    int track = 0;
    int score = 0;
    std::string score_formatted;
    std::string rank;
    int full_combo = 0;
    std::optional<std::string> full_combo_label;
    DifficultyLevel difficulty_level;
    TrackInfo song;
    Profile player;
};

struct PageLinks {
    std::string first;
    std::string last;
    std::optional<std::string> prev;
    std::optional<std::string> next;
};

template <typename T>
struct PagerPage {
    T data;
    PageLinks links;
};

class ApiClient {
public:
    ApiClient(std::string access_token, std::string base_url = "https://maitea.app");

    std::vector<Profile> get_profiles() const;
    std::vector<TrackInfo> get_tracks() const;
    PagerPage<std::vector<Play>> get_plays() const;
    PagerPage<std::vector<Play>> get_all_plays() const;
    std::vector<unsigned char> get_bytes(std::string_view url) const;

private:
    Json get_json(std::string_view path) const;
    PagerPage<std::vector<Play>> get_play_page(std::string_view path_or_url) const;

    std::string access_token_;
    std::string base_url_;
    HttpClient http_;
};

Image parse_image(const Json& json);
LocalizedName parse_localized_name(const Json& json);
TrackInfo parse_track(const Json& json);
DifficultyLevel parse_difficulty(const Json& json);
PlayStats parse_play_stats(const Json& json);
ProfileOptions parse_profile_options(const Json& json);
Profile parse_profile(const Json& json);
Play parse_play(const Json& json);
PageLinks parse_page_links(const Json& json);

} // namespace maifetch
