#pragma once

#include <iomanip>
#include <sstream>
#include "value.hpp"

namespace c2k::json {
    struct Number final : Value {
        double value;

        explicit Number(double const value = 0.0)
            : value{ value } {}

        [[nodiscard]] bool is_number() const override {
            return true;
        }

        [[nodiscard]] tl::optional<Number const&> as_number() const override {
            return *this;
        }

        [[nodiscard]] tl::optional<Number&> as_number() override {
            return *this;
        }

        [[nodiscard]] Utf8String format(usize, usize) const override {
            auto stream = std::ostringstream{};
            double intpart_;
            if (std::modf(value, &intpart_) == 0.0) {
                // this is an integer
                stream << std::fixed << std::setprecision(0) << value;
            } else {
                // this is a float
                stream << std::setprecision(8) << std::noshowpoint << value;
            }
            return std::move(stream).str();
        }
    };
}  // namespace c2k::json
