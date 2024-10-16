#pragma once

#include <string>
#include <variant>

namespace c2k::json {
    struct ParseError final {
        std::string message;
    };

    using Error = std::variant<ParseError>;
}  // namespace c2k::json
