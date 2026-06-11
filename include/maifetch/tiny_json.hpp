#pragma once

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace maifetch {

class Json {
public:
    using array = std::vector<Json>;
    using object = std::map<std::string, Json>;
    using value = std::variant<std::nullptr_t, bool, double, std::string, array, object>;

    Json();
    explicit Json(value value);

    [[nodiscard]] bool is_null() const;
    [[nodiscard]] bool is_array() const;
    [[nodiscard]] bool is_object() const;
    [[nodiscard]] const array& as_array() const;
    [[nodiscard]] const object& as_object() const;
    [[nodiscard]] std::string as_string(std::string fallback = "") const;
    [[nodiscard]] int as_int(int fallback = 0) const;
    [[nodiscard]] bool as_bool(bool fallback = false) const;
    [[nodiscard]] const Json& at(std::string_view key) const;
    [[nodiscard]] const Json& at(size_t index) const;

private:
    value value_;
};

Json parse_json(std::string_view source);
std::optional<Json> try_parse_json(std::string_view source);

} // namespace maifetch
