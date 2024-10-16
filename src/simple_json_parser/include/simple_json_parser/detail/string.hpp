#pragma once

#include <lib2k/utf8/string.hpp>
#include "value.hpp"

namespace c2k::json {
    struct String final : Value {
        Utf8String value;

        String() = default;

        String(Utf8String value)
            : value{ std::move(value) } {}

        [[nodiscard]] bool is_string() const override {
            return true;
        }

        [[nodiscard]] tl::optional<String const&> as_string() const override {
            return *this;
        }

        [[nodiscard]] tl::optional<String&> as_string() override {
            return *this;
        }

        [[nodiscard]] Utf8String format(usize, usize) const override {
            using namespace c2k::Utf8Literals;
            auto result = Utf8String{};
            result += '"';
            for (auto const c : value) {
                if (c == '"') {
                    result += "\\\"";
                } else if (c == '\\') {
                    result += "\\\\";
                } else if (c == '/') {
                    result += "\\/";
                } else if (c == '\b') {
                    result += "\\b";
                } else if (c == '\f') {
                    result += "\\f";
                } else if (c == '\n') {
                    result += "\\n";
                } else if (c == '\r') {
                    result += "\\r";
                } else if (c == '\t') {
                    result += "\\t";
                } else {
                    result += c;
                }
            }
            result += '"';
            return result;
        }
    };
}  // namespace c2k::json
