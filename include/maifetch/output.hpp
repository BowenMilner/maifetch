#pragma once

#include "maifetch/maitea.hpp"

#include <string>
#include <vector>

namespace maifetch {

std::string wide_to_normal(std::string_view value);
std::string difficulty_string(std::string_view diff);
std::string rank_string(std::string_view rank);
std::vector<std::string> create_info_strings(const Profile& profile, const std::vector<Play>& plays, int score_count);
void print_combined(const std::vector<std::string>& info_lines, const std::vector<std::string>& logo_lines, int logo_size);
void print_output(const std::vector<Play>& plays, const Profile& profile, int logo_size, int score_count, const ApiClient& client);

} // namespace maifetch
