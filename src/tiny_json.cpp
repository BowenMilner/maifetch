#include "maifetch/tiny_json.hpp"

#include <charconv>
#include <cctype>
#include <stdexcept>

namespace maifetch {
namespace {

const Json null_json;

class Parser {
public:
    explicit Parser(std::string_view source) : source_(source) {}

    Json parse() {
        skip_ws();
        auto value = parse_value();
        skip_ws();
        if (pos_ != source_.size()) {
            throw std::runtime_error("unexpected trailing JSON content");
        }
        return value;
    }

private:
    Json parse_value() {
        skip_ws();
        if (pos_ >= source_.size()) throw std::runtime_error("unexpected end of JSON");
        const char ch = source_[pos_];
        if (ch == 'n') return parse_literal("null", Json(nullptr));
        if (ch == 't') return parse_literal("true", Json(true));
        if (ch == 'f') return parse_literal("false", Json(false));
        if (ch == '"') return Json(parse_string());
        if (ch == '[') return Json(parse_array());
        if (ch == '{') return Json(parse_object());
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch))) return Json(parse_number());
        throw std::runtime_error("invalid JSON value");
    }

    Json parse_literal(std::string_view token, Json value) {
        if (source_.substr(pos_, token.size()) != token) {
            throw std::runtime_error("invalid JSON literal");
        }
        pos_ += token.size();
        return value;
    }

    std::string parse_string() {
        expect('"');
        std::string result;
        while (pos_ < source_.size()) {
            const char ch = source_[pos_++];
            if (ch == '"') return result;
            if (ch != '\\') {
                result += ch;
                continue;
            }
            if (pos_ >= source_.size()) throw std::runtime_error("unterminated JSON escape");
            const char escaped = source_[pos_++];
            switch (escaped) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u':
                    // Preserve non-ASCII escapes as a visible replacement; API fields used by maifetch are UTF-8 already.
                    pos_ += 4;
                    result += '?';
                    break;
                default:
                    throw std::runtime_error("unsupported JSON escape");
            }
        }
        throw std::runtime_error("unterminated JSON string");
    }

    double parse_number() {
        const size_t start = pos_;
        if (source_[pos_] == '-') ++pos_;
        while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) ++pos_;
        if (pos_ < source_.size() && source_[pos_] == '.') {
            ++pos_;
            while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) ++pos_;
        }
        if (pos_ < source_.size() && (source_[pos_] == 'e' || source_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < source_.size() && (source_[pos_] == '+' || source_[pos_] == '-')) ++pos_;
            while (pos_ < source_.size() && std::isdigit(static_cast<unsigned char>(source_[pos_]))) ++pos_;
        }
        double value = 0;
        const auto number = source_.substr(start, pos_ - start);
        const auto* first = number.data();
        const auto* last = first + number.size();
        const auto [ptr, ec] = std::from_chars(first, last, value);
        if (ec != std::errc() || ptr != last) throw std::runtime_error("invalid JSON number");
        return value;
    }

    Json::array parse_array() {
        Json::array result;
        expect('[');
        skip_ws();
        if (peek(']')) return result;
        while (true) {
            result.push_back(parse_value());
            skip_ws();
            if (peek(']')) return result;
            expect(',');
        }
    }

    Json::object parse_object() {
        Json::object result;
        expect('{');
        skip_ws();
        if (peek('}')) return result;
        while (true) {
            skip_ws();
            auto key = parse_string();
            skip_ws();
            expect(':');
            result.emplace(std::move(key), parse_value());
            skip_ws();
            if (peek('}')) return result;
            expect(',');
        }
    }

    void skip_ws() {
        while (pos_ < source_.size() && std::isspace(static_cast<unsigned char>(source_[pos_]))) ++pos_;
    }

    void expect(char ch) {
        skip_ws();
        if (pos_ >= source_.size() || source_[pos_] != ch) throw std::runtime_error("unexpected JSON token");
        ++pos_;
    }

    bool peek(char ch) {
        skip_ws();
        if (pos_ < source_.size() && source_[pos_] == ch) {
            ++pos_;
            return true;
        }
        return false;
    }

    std::string_view source_;
    size_t pos_ = 0;
};

} // namespace

Json::Json() : value_(nullptr) {}
Json::Json(value value) : value_(std::move(value)) {}

bool Json::is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool Json::is_array() const { return std::holds_alternative<array>(value_); }
bool Json::is_object() const { return std::holds_alternative<object>(value_); }

const Json::array& Json::as_array() const {
    if (const auto* value = std::get_if<array>(&value_)) return *value;
    throw std::runtime_error("JSON value is not an array");
}

const Json::object& Json::as_object() const {
    if (const auto* value = std::get_if<object>(&value_)) return *value;
    throw std::runtime_error("JSON value is not an object");
}

std::string Json::as_string(std::string fallback) const {
    if (const auto* value = std::get_if<std::string>(&value_)) return *value;
    return fallback;
}

int Json::as_int(int fallback) const {
    if (const auto* value = std::get_if<double>(&value_)) return static_cast<int>(*value);
    return fallback;
}

bool Json::as_bool(bool fallback) const {
    if (const auto* value = std::get_if<bool>(&value_)) return *value;
    return fallback;
}

const Json& Json::at(std::string_view key) const {
    if (const auto* object = std::get_if<Json::object>(&value_)) {
        const auto it = object->find(std::string(key));
        if (it != object->end()) return it->second;
    }
    return null_json;
}

const Json& Json::at(size_t index) const {
    if (const auto* array = std::get_if<Json::array>(&value_); array && index < array->size()) {
        return array->at(index);
    }
    return null_json;
}

Json parse_json(std::string_view source) {
    return Parser(source).parse();
}

std::optional<Json> try_parse_json(std::string_view source) {
    try {
        return parse_json(source);
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace maifetch
