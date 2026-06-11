#include "maifetch/config.hpp"
#include "maifetch/maitea.hpp"
#include "maifetch/output.hpp"

#include <future>
#include <iostream>

int main(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    maifetch::Config config;
    try {
        config = maifetch::load_config(args);
    } catch (const maifetch::HelpRequested&) {
        std::cout << maifetch::usage() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cout << error.what() << '\n';
        return 1;
    }

    try {
        const maifetch::ApiClient client(config.access_token, config.base_url);
        const auto profiles = client.get_profiles();
        if (profiles.empty()) {
            std::cout << "No profiles found\n";
            return 0;
        }

        std::cout << "Loading..." << std::flush;
        auto plays_future = std::async(std::launch::async, [&] { return client.get_plays().data; });
        if (plays_future.wait_for(std::chrono::seconds(30)) != std::future_status::ready) {
            std::cout << "\nAPI timed out\n";
            return 1;
        }
        std::cout << '\n';

        maifetch::print_output(plays_future.get(), profiles.front(), config.logo_size, config.score_count, client);
    } catch (const std::exception& error) {
        std::cout << error.what() << '\n';
        return 1;
    }
    return 0;
}
