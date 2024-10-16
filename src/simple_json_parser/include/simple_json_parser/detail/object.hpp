#pragma once

#include <concepts>
#include <ranges>
#include <stdexcept>
#include <vector>
#include "string.hpp"
#include "value.hpp"

namespace c2k::json {
    struct DuplicateKey final : std::runtime_error {
        explicit DuplicateKey(char const* const key)
            : std::runtime_error{ std::format("duplicate key: {}", key) } {}
    };

    struct Object final : Value {
        // stored as a vector to preserve insertion order
        std::vector<std::pair<String, ValuePointer>> values;

        Object() = default;

        explicit Object(std::convertible_to<std::pair<String, ValuePointer>> auto&&... key_value_pairs) {
            auto const num_pairs = sizeof...(key_value_pairs);
            values.reserve(num_pairs);
            (values.emplace_back(std::forward<decltype(key_value_pairs)>(key_value_pairs)), ...);
            // todo: improve as soon as Utf8String is hashable (and can be used in std::unordered_set)
            auto keys = std::vector<c2k::Utf8String>{};
            for (auto const& key : values | std::views::keys) {
                if (std::ranges::find(keys, key.value) != keys.end()) {
                    throw DuplicateKey{ key.value.c_str() };
                }
                keys.push_back(key.value);
            }
        }

        explicit Object(std::vector<std::pair<String, ValuePointer>> values)
            : values{ std::move(values) } {}

        [[nodiscard]] bool is_object() const override {
            return true;
        }

        [[nodiscard]] tl::optional<Object const&> as_object() const override {
            return *this;
        }

        [[nodiscard]] tl::optional<Object&> as_object() override {
            return *this;
        }

        [[nodiscard]] Utf8String format(usize const indentation_step, usize const base_indentation) const override {
            using namespace c2k::Utf8Literals;

            if (values.empty()) {
                return "{}"_utf8;
            }

            auto result = c2k::Utf8String{};
            result += "{\n"_utf8;
            for (auto const& [i, key_value_pair] : std::views::enumerate(values)) {
                auto const& [key, value] = key_value_pair;
                insert_indent(result, base_indentation + indentation_step);
                result += key.format(0, 0);
                result += ": "_utf8;
                result += value->format(indentation_step, base_indentation + indentation_step);
                if (static_cast<usize>(i) < values.size() - 1) {
                    result += ",\n"_utf8;
                } else {
                    result += '\n'_utf8;
                }
            }
            insert_indent(result, base_indentation);
            result += "}"_utf8;
            return result;
        }
    };
}  // namespace c2k::json
