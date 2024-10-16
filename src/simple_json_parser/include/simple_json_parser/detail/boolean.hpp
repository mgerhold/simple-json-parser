#pragma once

#include "value.hpp"

namespace c2k::json {
    struct Boolean final : Value {
        bool value;

        explicit Boolean(bool const value = false)
            : value{ value } {}

        [[nodiscard]] bool is_boolean() const override {
            return true;
        }

        [[nodiscard]] tl::optional<Boolean const&> as_boolean() const override {
            return *this;
        }

        [[nodiscard]] tl::optional<Boolean&> as_boolean() override {
            return *this;
        }

        [[nodiscard]] Utf8String format(usize, usize) const override {
            using namespace c2k::Utf8Literals;
            return value ? "true"_utf8 : "false"_utf8;
        }
    };
}  // namespace c2k::json
