#pragma once

#include "value.hpp"

namespace c2k::json {
    struct Null final : Value {
        [[nodiscard]] bool is_null() const override {
            return true;
        }

        [[nodiscard]] Utf8String format(usize, usize) const override {
            using namespace c2k::Utf8Literals;
            return "null"_utf8;
        }
    };
}  // namespace c2k::json
