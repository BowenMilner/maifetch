#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace maifetch {

struct Config {
    std::string access_token;
    int score_count = 4;
    int logo_size = 20;
    std::string base_url = "https://maitea.app";
};

struct CliOptions {
    std::optional<std::string> access_token;
    std::optional<int> score_count;
    std::optional<int> logo_size;
    std::optional<std::string> base_url;
    std::optional<std::filesystem::path> config_file;
};

struct HelpRequested : std::exception {
    [[nodiscard]] const char* what() const noexcept override { return "help requested"; }
};

CliOptions parse_cli(std::span<const std::string> args);
Config load_config(std::span<const std::string> args, const std::map<std::string, std::string>& env);
Config load_config(std::span<const std::string> args);
std::filesystem::path default_config_path(const std::map<std::string, std::string>& env);
std::string usage();

} // namespace maifetch
