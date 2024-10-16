#pragma once

#include <expected>
#include <filesystem>
#include <lib2k/utf8/string_view.hpp>
#include <simple_json_parser/detail/array.hpp>
#include <simple_json_parser/detail/boolean.hpp>
#include <simple_json_parser/detail/errors.hpp>
#include <simple_json_parser/detail/null.hpp>
#include <simple_json_parser/detail/number.hpp>
#include <simple_json_parser/detail/object.hpp>
#include <simple_json_parser/detail/string.hpp>
#include <simple_json_parser/detail/value.hpp>

namespace c2k::json {
    [[nodiscard]] std::expected<ValuePointer, Error> parse(
        Utf8StringView input,
        tl::optional<std::filesystem::path const&> path = tl::nullopt
    );
}
