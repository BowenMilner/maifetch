#include "test.hpp"

#include "maifetch/config.hpp"

#include <map>
#include <stdexcept>

namespace {

void cli_values_override_environment() {
    const std::vector<std::string> args = {"--access-token", "cli-token", "--score-count", "6", "--logo-size", "0"};
    const std::map<std::string, std::string> env = {
        {"MAITEA_TOKEN", "env-token"},
        {"MAITEA_SCORE_COUNT", "3"},
        {"MAITEA_LOGO_SIZE", "20"},
    };

    const auto config = maifetch::load_config(args, env);
    expect_eq(config.access_token, "cli-token", "CLI access token should win");
    expect_eq(config.score_count, 6, "CLI score count should win");
    expect_eq(config.logo_size, 0, "CLI logo size should win");
}

void score_count_is_capped() {
    try {
        const std::vector<std::string> args = {"--access-token", "token", "--score-count", "13"};
        (void)maifetch::load_config(args, {});
        expect_true(false, "score count above twelve should throw");
    } catch (const std::invalid_argument& error) {
        expect_eq(std::string(error.what()), "score count cannot be higher than 12", "score-count error text");
    }
}

void access_token_is_required() {
    try {
        const std::vector<std::string> args;
        (void)maifetch::load_config(args, {});
        expect_true(false, "missing access token should throw");
    } catch (const std::invalid_argument& error) {
        expect_eq(std::string(error.what()), "access token is required", "access-token error text");
    }
}

} // namespace

int main();

struct ConfigTests {
    ConfigTests() {
        cli_values_override_environment();
        score_count_is_capped();
        access_token_is_required();
    }
} config_tests;
