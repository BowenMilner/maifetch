#pragma once

#include <string>
#include <vector>

namespace maifetch {

std::vector<std::string> png_to_ascii(const std::vector<unsigned char>& png_bytes, int size);

} // namespace maifetch
