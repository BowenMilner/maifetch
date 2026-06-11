#include "maifetch/maitea.hpp"

#include <stdexcept>
#include <sstream>

namespace maifetch {
namespace {

std::string join_url(std::string base, std::string_view path_or_url) {
    if (path_or_url.starts_with("http://") || path_or_url.starts_with("https://")) return std::string(path_or_url);
    while (base.ends_with('/')) base.pop_back();
    if (!path_or_url.starts_with('/')) return base + "/" + std::string(path_or_url);
    return base + std::string(path_or_url);
}

std::map<std::string, std::string> auth_headers(const std::string& token) {
    return {
        {"Authorization", "Bearer " + token},
        {"Accept", "application/json"},
        {"Content-Type", "application/json"},
    };
}

std::string http_error(long status) {
    std::ostringstream out;
    out << "MaiTea API returned HTTP " << status;
    return out.str();
}

std::vector<Profile> parse_profiles(const Json& json) {
    std::vector<Profile> profiles;
    for (const auto& item : json.at("data").as_array()) profiles.push_back(parse_profile(item));
    return profiles;
}

std::vector<TrackInfo> parse_tracks(const Json& json) {
    std::vector<TrackInfo> tracks;
    for (const auto& item : json.at("data").as_array()) tracks.push_back(parse_track(item));
    return tracks;
}

std::vector<Play> parse_plays(const Json& json) {
    std::vector<Play> plays;
    for (const auto& item : json.at("data").as_array()) plays.push_back(parse_play(item));
    return plays;
}

} // namespace

ApiClient::ApiClient(std::string access_token, std::string base_url)
    : access_token_(std::move(access_token)), base_url_(std::move(base_url)) {}

std::vector<Profile> ApiClient::get_profiles() const {
    return parse_profiles(get_json("/api/v1/profiles"));
}

std::vector<TrackInfo> ApiClient::get_tracks() const {
    return parse_tracks(get_json("/api/v1/tracks"));
}

PagerPage<std::vector<Play>> ApiClient::get_plays() const {
    return get_play_page("/api/v1/plays");
}

PagerPage<std::vector<Play>> ApiClient::get_all_plays() const {
    return get_play_page("/api/v1/plays/all");
}

std::vector<unsigned char> ApiClient::get_bytes(std::string_view url) const {
    const auto response = http_.get(url, auth_headers(access_token_));
    if (response.status < 200 || response.status >= 300) {
        throw std::runtime_error(http_error(response.status));
    }
    return response.bytes;
}

Json ApiClient::get_json(std::string_view path) const {
    const auto response = http_.get(join_url(base_url_, path), auth_headers(access_token_));
    if (response.status < 200 || response.status >= 300) {
        throw std::runtime_error(http_error(response.status));
    }
    return parse_json(response.body);
}

PagerPage<std::vector<Play>> ApiClient::get_play_page(std::string_view path_or_url) const {
    const auto json = get_json(path_or_url);
    return {parse_plays(json), parse_page_links(json.at("links"))};
}

Image parse_image(const Json& json) {
    return {
        json.at("id").as_int(),
        json.at("png").as_string(),
        json.at("webp").as_string(),
    };
}

LocalizedName parse_localized_name(const Json& json) {
    return {json.at("en").as_string(), json.at("jp").as_string()};
}

TrackInfo parse_track(const Json& json) {
    return {
        json.at("id").as_int(),
        json.at("code").as_string(),
        parse_localized_name(json.at("name")),
        parse_localized_name(json.at("artist")),
    };
}

DifficultyLevel parse_difficulty(const Json& json) {
    return {
        json.at("key").as_int(),
        json.at("value").as_string(),
        json.at("label").as_string(),
    };
}

PlayStats parse_play_stats(const Json& json) {
    return {
        json.at("total").as_int(),
        json.at("wins").as_int(),
        json.at("vs").as_int(),
        json.at("sync").as_int(),
    };
}

ProfileOptions parse_profile_options(const Json& json) {
    return {
        parse_image(json.at("icon")),
        parse_image(json.at("icon_deka")),
    };
}

Profile parse_profile(const Json& json) {
    return {
        json.at("id").as_int(),
        json.at("name").as_string(),
        json.at("rating").as_int(),
        json.at("rating_highest").as_int(),
        json.at("level").as_int(),
        parse_play_stats(json.at("play_stats")),
        parse_profile_options(json.at("options")),
    };
}

Play parse_play(const Json& json) {
    std::optional<std::string> full_combo_label;
    if (!json.at("full_combo_label").is_null()) full_combo_label = json.at("full_combo_label").as_string();
    return {
        json.at("id").as_int(),
        json.at("achievement").as_int(),
        json.at("achievement_formatted").as_string(),
        json.at("track").as_int(),
        json.at("score").as_int(),
        json.at("score_formatted").as_string(),
        json.at("rank").as_string(),
        json.at("full_combo").as_int(),
        full_combo_label,
        parse_difficulty(json.at("difficulty_level")),
        parse_track(json.at("song")),
        parse_profile(json.at("player")),
    };
}

PageLinks parse_page_links(const Json& json) {
    auto optional_string = [&](std::string_view key) -> std::optional<std::string> {
        if (json.at(key).is_null()) return std::nullopt;
        return json.at(key).as_string();
    };
    return {
        json.at("first").as_string(),
        json.at("last").as_string(),
        optional_string("prev"),
        optional_string("next"),
    };
}

} // namespace maifetch
