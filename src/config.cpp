#include "maifetch/config.hpp"
#include "maifetch/tiny_json.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>

namespace maifetch {
namespace {

std::optional<int> parse_int(std::optional<std::string> value) {
    if (!value) return std::nullopt;
    try {
        return std::stoi(*value);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> env_value(const std::map<std::string, std::string>& env, std::string_view key) {
    const auto it = env.find(std::string(key));
    if (it == env.end()) return std::nullopt;
    return it->second;
}

template <typename T>
std::optional<T> first_present(std::initializer_list<std::optional<T>> values) {
    for (auto& value : values) {
        if (value) return value;
    }
    return std::nullopt;
}

std::map<std::string, std::string> current_env() {
    std::map<std::string, std::string> result;
    for (const auto* key : {"MAITEA_TOKEN", "MAITEA_SCORE_COUNT", "MAITEA_LOGO_SIZE", "MAITEA_CONFIG_FILE",
                            "MAITEA_BASE_URL", "MAIFETCH_TOKEN", "MAIFETCH_SCORE_COUNT", "MAIFETCH_LOGO_SIZE",
                            "MAIFETCH_CONFIG_FILE", "MAIFETCH_BASE_URL", "APPDATA", "XDG_CONFIG_HOME"}) {
        if (const char* value = std::getenv(key)) result.emplace(key, value);
    }
    return result;
}

Config read_config_file(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return {};
    std::ifstream input(path);
    if (!input) throw std::runtime_error("could not read config file: " + path.string());
    const std::string body((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto json = parse_json(body);

    Config config;
    config.access_token = json.at("accessToken").as_string();
    config.score_count = json.at("scoreCount").as_int(config.score_count);
    config.logo_size = json.at("logoSize").as_int(config.logo_size);
    config.base_url = json.at("baseUrl").as_string(config.base_url);
    return config;
}

} // namespace

CliOptions parse_cli(std::span<const std::string> args) {
    CliOptions options;
    for (size_t index = 0; index < args.size(); ++index) {
        const auto& arg = args[index];
        const auto next_value = [&]() -> std::string {
            if (index + 1 >= args.size()) throw std::invalid_argument("missing value for " + arg);
            ++index;
            return args[index];
        };

        if (arg == "--access-token" || arg == "-a" || arg == "-t") options.access_token = next_value();
        else if (arg == "--score-count" || arg == "-s") options.score_count = std::stoi(next_value());
        else if (arg == "--logo-size" || arg == "-l") options.logo_size = std::stoi(next_value());
        else if (arg == "--config-file" || arg == "-c") options.config_file = next_value();
        else if (arg == "--base-url") options.base_url = next_value();
        else if (arg == "--help" || arg == "-h") throw HelpRequested();
        else throw std::invalid_argument("unknown argument: " + arg);
    }
    return options;
}

Config load_config(std::span<const std::string> args, const std::map<std::string, std::string>& env) {
    const auto cli = parse_cli(args);
    std::optional<std::filesystem::path> env_config_file;
    if (const auto maitea = env_value(env, "MAITEA_CONFIG_FILE")) env_config_file = *maitea;
    else if (const auto maifetch = env_value(env, "MAIFETCH_CONFIG_FILE")) env_config_file = *maifetch;
    const auto config_path = cli.config_file ? *cli.config_file : env_config_file.value_or(default_config_path(env));
    const auto file = read_config_file(config_path);

    Config merged;
    merged.access_token = first_present<std::string>({cli.access_token, env_value(env, "MAITEA_TOKEN"), env_value(env, "MAIFETCH_TOKEN")}).value_or(file.access_token);
    merged.score_count = first_present<int>({cli.score_count, parse_int(env_value(env, "MAITEA_SCORE_COUNT")), parse_int(env_value(env, "MAIFETCH_SCORE_COUNT"))}).value_or(file.score_count);
    merged.logo_size = first_present<int>({cli.logo_size, parse_int(env_value(env, "MAITEA_LOGO_SIZE")), parse_int(env_value(env, "MAIFETCH_LOGO_SIZE"))}).value_or(file.logo_size);
    merged.base_url = first_present<std::string>({cli.base_url, env_value(env, "MAITEA_BASE_URL"), env_value(env, "MAIFETCH_BASE_URL")}).value_or(file.base_url);

    if (merged.access_token.empty()) throw std::invalid_argument("access token is required");
    if (merged.score_count > 12) throw std::invalid_argument("score count cannot be higher than 12");
    return merged;
}

Config load_config(std::span<const std::string> args) {
    return load_config(args, current_env());
}

std::filesystem::path default_config_path(const std::map<std::string, std::string>& env) {
#ifdef _WIN32
    if (const auto appdata = env_value(env, "APPDATA")) return std::filesystem::path(*appdata) / "maifetch.json";
    return std::filesystem::path(".") / "maifetch.json";
#elif __APPLE__
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / "Library" / "Application Support" / "maifetch.json";
#else
    if (const auto xdg = env_value(env, "XDG_CONFIG_HOME")) return std::filesystem::path(*xdg) / "maifetch.json";
    const char* home = std::getenv("HOME");
    return std::filesystem::path(home ? home : ".") / ".config" / "maifetch.json";
#endif
}

std::string usage() {
    return R"(Usage: maifetch [options]

Options:
  -a, -t, --access-token <token>  Access token for the MaiTea account
  -s, --score-count <count>       Amount of recent scores to view (max 12)
  -l, --logo-size <size>          Size of the ASCII logo (<1 disables)
  -c, --config-file <path>        JSON config file to use
      --base-url <url>            MaiTea API base URL
  -h, --help                      Show this help)";
}

} // namespace maifetch
