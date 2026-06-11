#include "maifetch/output.hpp"

#include "maifetch/ansi.hpp"
#include "maifetch/ascii_image.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace maifetch {
namespace {

std::string accent(std::string_view value) {
    return fg(value, 72, 184, 200);
}

std::string rating_value(int value) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << (static_cast<double>(value) / 100.0);
    return out.str();
}

template <typename T>
std::string to_string_value(const T& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

} // namespace

std::string wide_to_normal(std::string_view value) {
    std::string result;
    for (size_t i = 0; i < value.size();) {
        const unsigned char a = value[i];
        if (i + 2 < value.size() && a == 0xEF) {
            const unsigned char b = value[i + 1];
            const unsigned char c = value[i + 2];
            if (b == 0xBC && c >= 0x81 && c <= 0xBF) {
                result += static_cast<char>(c - 0x60);
                i += 3;
                continue;
            }
            if (b == 0xBD && c >= 0x80 && c <= 0x9E) {
                result += static_cast<char>(c - 0x20);
                i += 3;
                continue;
            }
        }
        result += static_cast<char>(a);
        ++i;
    }
    return result;
}

std::string difficulty_string(std::string_view diff) {
    if (diff == "easy") return color("Easy", 255, 255, 255, 69, 174, 255);
    if (diff == "basic") return color("Basic", 255, 255, 255, 111, 212, 61);
    if (diff == "advanced") return color("Advanced", 255, 255, 255, 248, 183, 9);
    if (diff == "expert") return color("Expert", 255, 255, 255, 255, 46, 66);
    if (diff == "master") return color("Master", 255, 255, 255, 171, 140, 233);
    if (diff == "remaster" || diff == "re:master") return color("Re:Master", 255, 255, 255, 207, 114, 237);
    if (diff == "utage") return color("Utage", 255, 255, 255, 255, 68, 1);
    return std::string(diff);
}

std::string rank_string(std::string_view rank) {
    if (rank == "SSS+") return fg("S", 255, 200, 54) + fg("S", 225, 38, 165) + fg("S", 73, 64, 233) + fg("+", 21, 203, 148);
    if (rank == "SSS") return fg("S", 255, 200, 54) + fg("S", 232, 39, 148) + fg("S", 18, 195, 144);
    if (rank == "SS+" || rank == "SS") return color(rank, 248, 200, 75, 143, 71, 33);
    if (rank == "S+" || rank == "S") return color(rank, 248, 200, 75, 75, 82, 82);
    if (rank == "AAA" || rank == "AA" || rank == "A") return fg(rank, 23, 163, 255);
    return std::string(rank);
}

std::vector<std::string> create_info_strings(const Profile& profile, const std::vector<Play>& plays, int score_count) {
    const auto name = wide_to_normal(profile.name);
    std::vector<std::string> lines = {
        accent(name),
        std::string(name.size(), '-'),
        accent("ID") + ": " + to_string_value(profile.id),
        accent("Rating") + ": " + rating_value(profile.rating) + " / " + rating_value(profile.rating_highest),
        accent("Level") + ": " + to_string_value(profile.level),
        accent("Total Credits") + ": " + to_string_value(profile.play_stats.total),
        accent("Recent Scores") + ":",
    };

    const auto count = std::min<size_t>(std::max(score_count, 0), plays.size());
    for (size_t index = 0; index < count; ++index) {
        const auto& play = plays[index];
        lines.push_back("  " + play.song.name.en + "  " + difficulty_string(play.difficulty_level.value));
        lines.push_back("  " + play.score_formatted + " " + play.achievement_formatted + "% " +
                        rank_string(play.rank) + " " + play.full_combo_label.value_or(""));
        lines.emplace_back();
    }
    return lines;
}

void print_combined(const std::vector<std::string>& info_lines, const std::vector<std::string>& logo_lines, int logo_size) {
    const auto max_length = std::max(info_lines.size(), logo_lines.size());
    const std::string blank_logo(static_cast<size_t>(std::max(logo_size * 2, 0)), ' ');
    for (size_t index = 0; index < max_length; ++index) {
        const auto& logo = index < logo_lines.size() ? logo_lines[index] : blank_logo;
        const auto& info = index < info_lines.size() ? info_lines[index] : std::string();
        std::cout << logo << "  " << info << '\n';
    }
}

void print_output(const std::vector<Play>& plays, const Profile& profile, int logo_size, int score_count, const ApiClient& client) {
    const auto info_lines = create_info_strings(profile, plays, score_count);
    if (logo_size <= 0 || profile.options.icon.png.empty()) {
        for (const auto& line : info_lines) std::cout << line << '\n';
        return;
    }

    const auto icon_bytes = client.get_bytes(profile.options.icon.png);
    const auto logo_lines = png_to_ascii(icon_bytes, logo_size);
    print_combined(info_lines, logo_lines, logo_size);
}

} // namespace maifetch
