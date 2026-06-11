#pragma once

#include <sstream>
#include <string>

namespace maifetch {

inline std::string fg(std::string_view text, int r, int g, int b) {
    std::ostringstream out;
    out << "\x1b[38;2;" << r << ';' << g << ';' << b << 'm' << text << "\x1b[0m";
    return out.str();
}

inline std::string color(std::string_view text, int fg_r, int fg_g, int fg_b, int bg_r, int bg_g, int bg_b) {
    std::ostringstream out;
    out << "\x1b[38;2;" << fg_r << ';' << fg_g << ';' << fg_b << 'm'
        << "\x1b[48;2;" << bg_r << ';' << bg_g << ';' << bg_b << 'm'
        << text << "\x1b[0m";
    return out.str();
}

} // namespace maifetch
