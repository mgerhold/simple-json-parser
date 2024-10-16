#pragma once

#include <concepts>
#include <ranges>
#include <vector>
#include "value.hpp"

namespace c2k::json {
    struct Array final : Value {
        std::vector<ValuePointer> elements;

        Array() = default;

        explicit Array(std::convertible_to<ValuePointer> auto&&... elements) {
            auto const num_elements = sizeof...(elements);
            this->elements.reserve(num_elements);
            (this->elements.emplace_back(std::forward<decltype(elements)>(elements)), ...);
        }

        explicit Array(std::vector<ValuePointer> elements)
            : elements{ std::move(elements) } {}

        [[nodiscard]] bool is_array() const override {
            return true;
        }

        [[nodiscard]] tl::optional<Array const&> as_array() const override {
            return *this;
        }

        [[nodiscard]] tl::optional<Array&> as_array() override {
            return *this;
        }

        [[nodiscard]] Utf8String format(usize const indentation_step, usize const base_indentation) const override {
            using namespace c2k::Utf8Literals;

            if (elements.empty()) {
                return "[]"_utf8;
            }

            auto result = Utf8String{};
            result += "[\n"_utf8;
            for (auto const& [i, element] : std::views::enumerate(elements)) {
                insert_indent(result, base_indentation + indentation_step);
                result += element->format(indentation_step, base_indentation + indentation_step);
                if (static_cast<usize>(i) < elements.size() - 1) {
                    result += ",\n"_utf8;
                } else {
                    result += '\n'_utf8;
                }
            }
            insert_indent(result, base_indentation);
            result += ']';
            return result;
        }
    };
}  // namespace c2k::json
