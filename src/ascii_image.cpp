#include "maifetch/ascii_image.hpp"

#include "maifetch/ansi.hpp"

#if MAIFETCH_WITH_PNG
#include <png.h>
#endif

#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace maifetch {
namespace {

constexpr std::string_view ramp = "@%#*+=-:. ";

#if MAIFETCH_WITH_PNG
struct PngBuffer {
    const std::vector<unsigned char>* bytes = nullptr;
    size_t offset = 0;
};

void read_png_data(png_structp png, png_bytep data, png_size_t length) {
    auto* buffer = static_cast<PngBuffer*>(png_get_io_ptr(png));
    if (!buffer || buffer->offset + length > buffer->bytes->size()) png_error(png, "unexpected end of PNG");
    std::copy_n(buffer->bytes->data() + buffer->offset, length, data);
    buffer->offset += length;
}

struct Pixel {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};

std::vector<Pixel> decode_png(const std::vector<unsigned char>& png_bytes, int& width, int& height) {
    if (png_bytes.size() < 8 || png_sig_cmp(const_cast<png_bytep>(png_bytes.data()), 0, 8) != 0) {
        throw std::runtime_error("profile icon is not a PNG");
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png) throw std::runtime_error("could not create PNG reader");
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        throw std::runtime_error("could not create PNG info");
    }
    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, nullptr);
        throw std::runtime_error("could not decode PNG");
    }

    PngBuffer buffer{&png_bytes, 0};
    png_set_read_fn(png, &buffer, read_png_data);
    png_read_info(png, info);

    width = static_cast<int>(png_get_image_width(png, info));
    height = static_cast<int>(png_get_image_height(png, info));
    const auto color_type = png_get_color_type(png, info);
    const auto bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);
    if (!(color_type & PNG_COLOR_MASK_ALPHA)) png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);

    png_read_update_info(png, info);
    std::vector<unsigned char> raw(static_cast<size_t>(png_get_rowbytes(png, info)) * height);
    std::vector<png_bytep> rows(static_cast<size_t>(height));
    for (int y = 0; y < height; ++y) rows[static_cast<size_t>(y)] = raw.data() + static_cast<size_t>(y) * png_get_rowbytes(png, info);
    png_read_image(png, rows.data());
    png_destroy_read_struct(&png, &info, nullptr);

    std::vector<Pixel> pixels(static_cast<size_t>(width * height));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto* px = rows[static_cast<size_t>(y)] + static_cast<size_t>(x) * 4;
            pixels[static_cast<size_t>(y * width + x)] = {px[0], px[1], px[2]};
        }
    }
    return pixels;
}
#endif

} // namespace

std::vector<std::string> png_to_ascii(const std::vector<unsigned char>& png_bytes, int size) {
#if MAIFETCH_WITH_PNG
    int source_width = 0;
    int source_height = 0;
    const auto pixels = decode_png(png_bytes, source_width, source_height);
    const int width = std::max(1, size * 2);
    const int height = std::max(1, size);

    std::vector<std::string> lines;
    lines.reserve(static_cast<size_t>(height));
    for (int y = 0; y < height; ++y) {
        std::string line;
        for (int x = 0; x < width; ++x) {
            const int src_x = std::min(source_width - 1, static_cast<int>(std::floor((x + 0.5) * source_width / width)));
            const int src_y = std::min(source_height - 1, static_cast<int>(std::floor((y + 0.5) * source_height / height)));
            const auto pixel = pixels[static_cast<size_t>(src_y * source_width + src_x)];
            const auto luminance = static_cast<int>(0.2126 * pixel.r + 0.7152 * pixel.g + 0.0722 * pixel.b);
            const char glyph = ramp[static_cast<size_t>(luminance * (ramp.size() - 1) / 255)];
            line += fg(std::string(1, glyph == ' ' ? '#' : glyph), pixel.r, pixel.g, pixel.b);
        }
        lines.push_back(std::move(line));
    }
    return lines;
#else
    const int width = std::max(1, size * 2);
    const int height = std::max(1, size);
    std::vector<std::string> lines;
    lines.reserve(static_cast<size_t>(height));
    const auto n = std::max<size_t>(png_bytes.size(), 1);
    for (int y = 0; y < height; ++y) {
        std::string line;
        for (int x = 0; x < width; ++x) {
            const auto index = (static_cast<size_t>(y) * width + x) % n;
            const auto seed = png_bytes.empty() ? 120 : png_bytes[index];
            const int r = static_cast<int>((seed * 37 + x * 11) % 256);
            const int g = static_cast<int>((seed * 53 + y * 17) % 256);
            const int b = static_cast<int>((seed * 97 + x * y) % 256);
            const int luminance = static_cast<int>(0.2126 * r + 0.7152 * g + 0.0722 * b);
            const char glyph = ramp[static_cast<size_t>(luminance * (ramp.size() - 1) / 255)];
            line += fg(std::string(1, glyph == ' ' ? '#' : glyph), r, g, b);
        }
        lines.push_back(std::move(line));
    }
    return lines;
#endif
}

} // namespace maifetch
