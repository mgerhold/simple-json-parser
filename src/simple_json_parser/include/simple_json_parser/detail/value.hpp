#pragma once

#include <lib2k/types.hpp>
#include <lib2k/utf8/string.hpp>
#include <memory>
#include <tl/optional.hpp>

namespace c2k::json {
    struct Array;
    struct Boolean;
    struct Null;
    struct Number;
    struct Object;
    struct String;

    struct Value {
        Value() = default;
        Value(Value const& other) = default;
        Value(Value&& other) noexcept = default;
        Value& operator=(Value const& other) = default;
        Value& operator=(Value&& other) noexcept = default;
        virtual ~Value() = default;

        [[nodiscard]] virtual bool is_object() const {
            return false;
        }

        [[nodiscard]] virtual bool is_array() const {
            return false;
        }

        [[nodiscard]] virtual bool is_string() const {
            return false;
        }

        [[nodiscard]] virtual bool is_number() const {
            return false;
        }

        [[nodiscard]] virtual bool is_boolean() const {
            return false;
        }

        [[nodiscard]] virtual bool is_null() const {
            return false;
        }

        [[nodiscard]] virtual tl::optional<Object const&> as_object() const {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Object&> as_object() {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Array const&> as_array() const {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Array&> as_array() {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<String const&> as_string() const {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<String&> as_string() {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Number const&> as_number() const {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Number&> as_number() {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Boolean const&> as_boolean() const {
            return tl::nullopt;
        }

        [[nodiscard]] virtual tl::optional<Boolean&> as_boolean() {
            return tl::nullopt;
        }

        [[nodiscard]] Utf8String pretty_print(usize const indentation_step = 2) const {
            return format(indentation_step, 0);
        }

        [[nodiscard]] virtual Utf8String format(usize indentation_step, usize base_indentation) const = 0;

    protected:
        static void insert_indent(Utf8String& string, usize const amount) {
            using namespace c2k::Utf8Literals;
            for (auto i = usize{ 0 }; i < amount; ++i) {
                string += ' '_utf8;
            }
        }
    };

    using ValuePointer = std::unique_ptr<Value>;
}  // namespace c2k::json
