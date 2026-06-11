#include "test.hpp"

#include "maifetch/output.hpp"

namespace {

void info_strings_keep_profile_and_recent_score_content() {
    maifetch::Profile profile;
    profile.id = 123;
    profile.name = "\xEF\xBC\xB4\xEF\xBD\x85\xEF\xBD\x93\xEF\xBD\x94";
    profile.rating = 1500;
    profile.rating_highest = 1600;
    profile.level = 42;
    profile.play_stats.total = 99;

    maifetch::Play play;
    play.score_formatted = "1,000,000";
    play.achievement_formatted = "100.0000";
    play.rank = "SSS+";
    play.full_combo_label = "FC";
    play.difficulty_level.value = "master";
    play.song.name.en = "Song";

    const auto lines = maifetch::create_info_strings(profile, {play}, 1);
    std::string joined;
    for (const auto& line : lines) joined += line + "\n";

    expect_true(joined.find("Test") != std::string::npos, "full-width profile name should normalize");
    expect_true(joined.find("ID") != std::string::npos && joined.find("123") != std::string::npos, "profile id should print");
    expect_true(joined.find("1,000,000") != std::string::npos, "score should print");
    expect_true(joined.find("100.0000%") != std::string::npos, "achievement should print");
    expect_true(joined.find("Song") != std::string::npos, "song name should print");
}

} // namespace

struct OutputTests {
    OutputTests() {
        info_strings_keep_profile_and_recent_score_content();
    }
} output_tests;
