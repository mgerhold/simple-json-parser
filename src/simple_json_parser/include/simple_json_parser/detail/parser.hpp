#pragma once

#include <algorithm>
#include <array>
#include <expected>
#include <format>
#include <lib2k/static_vector.hpp>
#include <lib2k/types.hpp>
#include <lib2k/utf8/string_view.hpp>
#include <simple_json_parser/detail/array.hpp>
#include <simple_json_parser/detail/boolean.hpp>
#include <simple_json_parser/detail/errors.hpp>
#include <simple_json_parser/detail/null.hpp>
#include <simple_json_parser/detail/number.hpp>
#include <simple_json_parser/detail/object.hpp>
#include <simple_json_parser/detail/string.hpp>
#include <simple_json_parser/detail/value.hpp>
#include <string>

[[nodiscard]] inline std::expected<i32, c2k::json::Error> convert_surrogates_to_codepoint(
    u16 const high_surrogate,
    u16 const low_surrogate
) {
    if (high_surrogate < 0xD800 or high_surrogate > 0xDBFF or low_surrogate < 0xDC00 or low_surrogate > 0xDFFF) {
        return std::unexpected{ c2k::json::ParseError{ "invalid surrogate pair" } };
    }
    return 0x10000 + ((high_surrogate - 0xD800) << 10) + (low_surrogate - 0xDC00);
}

namespace c2k::json::detail {
    class Parser final {
        Utf8StringView m_input;
        Utf8StringView::ConstIterator m_current;

    public:
        explicit Parser(Utf8StringView const input)
            : m_input{ input }, m_current{ input.cbegin() } {}

        [[nodiscard]] std::expected<ValuePointer, Error> parse() {
            return element();
        }

    private:
        [[nodiscard]] std::expected<ValuePointer, Error> element() {
            consume_whitespace();
            auto result = value();
            consume_whitespace();
            return result;
        }

        [[nodiscard]] std::expected<ValuePointer, Error> value() {
            auto const c = current().as_string_view().front();

            switch (c) {
                case '{':
                    return object();
                case '[':
                    return array();
                case '"':
                    return string();
                case 't':
                case 'f':
                    return boolean();
                case 'n':
                    return null();
                default:
                    if (c != '-' and not std::isdigit(static_cast<unsigned char>(c))) {
                        if (is_at_end_of_input()) {
                            return std::unexpected{ ParseError{ "unexpected end of input" } };
                        }
                        return std::unexpected{ ParseError{
                            std::format("unexpected character: {}", current().as_string_view()) } };
                    }
                    return number();
            }
        }

        [[nodiscard]] std::expected<ValuePointer, Error> object() {
            if (auto const result = consume('{'); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            consume_whitespace();
            auto members = std::vector<std::pair<String, ValuePointer>>{};
            if (current() != '}') {
                auto members_result = this->members();
                if (members_result.has_value()) {
                    members = std::move(members_result).value();
                } else {
                    return std::unexpected{ members_result.error() };
                }
            }
            if (auto const result = consume('}'); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            return std::make_unique<Object>(std::move(members));
        }

        [[nodiscard]] std::expected<std::vector<std::pair<String, ValuePointer>>, Error> members() {
            auto result = std::vector<std::pair<String, ValuePointer>>{};
            auto member_result = member();
            if (not member_result.has_value()) {
                return std::unexpected{ member_result.error() };
            }
            result.emplace_back(std::move(member_result).value());
            if (current() != ',') {
                return result;
            }
            advance();  // consume ','
            auto sub_members_result = members();
            if (not sub_members_result.has_value()) {
                return std::unexpected{ sub_members_result.error() };
            }
            auto sub_members = std::move(sub_members_result).value();
            std::ranges::move(sub_members, std::back_inserter(result));
            return result;
        }

        [[nodiscard]] std::expected<std::pair<String, ValuePointer>, Error> member() {
            consume_whitespace();
            auto key_result = string();
            if (not key_result.has_value()) {
                return std::unexpected{ key_result.error() };
            }
            auto key = std::move(key_result).value();
            auto key_string = std::move(dynamic_cast<String&>(*key));
            consume_whitespace();
            if (current() != ':') {
                return std::unexpected{ ParseError{ "expected ':'" } };
            }
            advance();  // consume ':'
            auto element_result = element();
            if (not element_result.has_value()) {
                return std::unexpected{ element_result.error() };
            }
            return std::make_pair(std::move(key_string), std::move(element_result).value());
        }

        [[nodiscard]] std::expected<ValuePointer, Error> array() {
            if (auto const result = consume('['); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            consume_whitespace();
            auto elements = std::vector<ValuePointer>{};
            if (current() != ']') {
                auto elements_result = this->elements();
                if (elements_result.has_value()) {
                    elements = std::move(elements_result).value();
                } else {
                    return std::unexpected{ elements_result.error() };
                }
            }
            if (auto const result = consume(']'); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            return std::make_unique<Array>(std::move(elements));
        }

        [[nodiscard]] std::expected<std::vector<ValuePointer>, Error> elements() {
            auto result = std::vector<ValuePointer>{};
            auto element_result = element();
            if (not element_result.has_value()) {
                return std::unexpected{ element_result.error() };
            }
            result.emplace_back(std::move(element_result).value());
            if (current() != ',') {
                return result;
            }
            advance();  // consume ','
            auto sub_elements_result = elements();
            if (not sub_elements_result.has_value()) {
                return std::unexpected{ sub_elements_result.error() };
            }
            auto sub_elements = std::move(sub_elements_result).value();
            std::ranges::move(sub_elements, std::back_inserter(result));
            return result;
        }

        [[nodiscard]] std::expected<ValuePointer, Error> string() {
            using namespace c2k::Utf8Literals;
            if (auto const result = consume('"'); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            if (current() == '"') {
                advance();  // consume '"'
                return std::make_unique<String>(""_utf8);
            }
            static constexpr auto is_character = [](Utf8Char const c) {
                auto const codepoint = c.codepoint();
                return codepoint >= 0x20 and codepoint <= 0x10FFFF and codepoint != '"' and codepoint != '\\';
            };
            auto result = Utf8String{};
            while (not is_at_end_of_input() and current() != '"') {
                if (current() == '"') {
                    advance();
                    break;
                }
                if (current() == '\\') {
                    auto const escape_sequence_result = escape_sequence();
                    if (not escape_sequence_result.has_value()) {
                        return std::unexpected{ escape_sequence_result.error() };
                    }
                    result += escape_sequence_result.value();
                    continue;
                }
                if (is_character(current())) {
                    result += current();
                    advance();
                    continue;
                }
                return std::unexpected{ ParseError{
                    std::format("invalid character in string: {}", current().as_string_view()) } };
            }
            if (current() != '"') {
                return std::unexpected{ ParseError{ "expected '\"'" } };
            }
            advance();  // consume '"'
            return std::make_unique<String>(std::move(result));
        }

        [[nodiscard]] std::expected<Utf8Char, Error> escape_sequence() {
            if (auto const result = consume('\\'); not result.has_value()) {
                return std::unexpected{ result.error() };
            }
            if (is_at_end_of_input()) {
                return std::unexpected{ ParseError{ "unexpected end of input" } };
            }
            auto const c = current().as_string_view().front();
            switch (c) {
                case '"':
                case '\\':
                case '/':
                    advance();
                    return c;
                case 'b':
                    advance();
                    return '\b';
                case 'f':
                    advance();
                    return '\f';
                case 'n':
                    advance();
                    return '\n';
                case 'r':
                    advance();
                    return '\r';
                case 't':
                    advance();
                    return '\t';
                case 'u': {
                    auto escape_sequences = c2k::StaticVector<u16, 2>{};
                    advance();
                    auto const result = unicode_escape_sequence();
                    if (not result.has_value()) {
                        return std::unexpected{ result.error() };
                    }
                    escape_sequences.push_back(result.value());
                    if (current() == '\\' and peek() == 'u') {
                        advance();
                        advance();
                        auto const second_result = unicode_escape_sequence();
                        if (not second_result.has_value()) {
                            return std::unexpected{ second_result.error() };
                        }
                        escape_sequences.push_back(second_result.value());
                    }
                    if (escape_sequences.size() == 1) {
                        if (escape_sequences.front() >= 0xD800 and escape_sequences.front() <= 0xDFFF) {
                            return std::unexpected{ ParseError{ "invalid unicode escape sequence" } };
                        }
                        auto const codepoint_result = Utf8Char::from_codepoint(escape_sequences.front());
                        if (not codepoint_result.has_value()) {
                            return std::unexpected{ ParseError{ "invalid unicode escape sequence" } };
                        }
                        return codepoint_result.value();
                    }
                    assert(escape_sequences.size() == 2);
                    auto const high_surrogate = escape_sequences.front();
                    auto const low_surrogate = escape_sequences.back();
                    auto const codepoint_result = convert_surrogates_to_codepoint(high_surrogate, low_surrogate);
                    if (not codepoint_result.has_value()) {
                        return std::unexpected{ codepoint_result.error() };
                    }
                    return Utf8Char::from_codepoint(codepoint_result.value()).value();
                }
                default:
                    return std::unexpected{ ParseError{ "invalid escape sequence" } };
            }
        }

        [[nodiscard]] std::expected<u16, Error> unicode_escape_sequence() {
            auto hex_string = std::array<char, 4>{};
            for (auto& c : hex_string) {
                auto const hex_result = hex();
                if (not hex_result.has_value()) {
                    return std::unexpected{ hex_result.error() };
                }
                c = hex_result.value();
            }
            auto escape_sequence = u16{};
            auto const conversion_result =
                std::from_chars(hex_string.data(), hex_string.data() + hex_string.size(), escape_sequence, 16);
            if (conversion_result.ec != std::errc{} or conversion_result.ptr != hex_string.data() + hex_string.size()) {
                return std::unexpected{ ParseError{ "error converting hex constant" } };
            }
            return escape_sequence;
        }

        [[nodiscard]] std::expected<char, Error> hex() {
            static constexpr auto is_ascii = [](char const c) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
                return (std::is_signed_v<decltype(c)> and c >= 0) or (std::is_unsigned_v<decltype(c)> and c <= 127);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
            };

            auto const c = current().as_string_view().front();
            if (not is_ascii(c) or not std::isxdigit(static_cast<unsigned char>(c))) {
                return std::unexpected{ ParseError{ "invalid hex digit" } };
            }
            advance();
            return c;
        }

        [[nodiscard]] std::expected<ValuePointer, Error> number() {
            auto const start_iterator = m_current;
            auto integer_result = integer();
            if (not integer_result.has_value()) {
                return std::unexpected{ integer_result.error() };
            }
            if (current() != '.') {
                return std::make_unique<Number>(static_cast<double>(integer_result.value()));
            }
            advance();  // consume '.'
            auto const fractional_part_start_iterator = m_current;
            while (std::isdigit(static_cast<unsigned char>(current().as_string_view().front()))) {
                advance();
            }
            auto const end_iterator = m_current;
            if (fractional_part_start_iterator == end_iterator) {
                return std::unexpected{ ParseError{ "expected digit" } };
            }
            auto const number_string_view = Utf8StringView{ start_iterator, end_iterator };
            auto conversion_buffer = std::string{};
            conversion_buffer.reserve(number_string_view.num_bytes());
            for (auto const c : number_string_view) {
                conversion_buffer.push_back(c.as_string_view().front());
            }
            auto result = double{};
            auto const conversion_result =
                std::from_chars(conversion_buffer.data(), conversion_buffer.data() + conversion_buffer.size(), result);
            if (conversion_result.ec != std::errc{}
                or conversion_result.ptr != conversion_buffer.data() + conversion_buffer.size()) {
                return std::unexpected{ ParseError{ "number out of range" } };
            }
            return std::make_unique<Number>(result);
        }

        [[nodiscard]] std::expected<i64, Error> integer(bool const allow_negative = true) {
            if (current() == '0') {
                advance();  // consume '0'
                return 0;
            }

            auto const starts_with_minus = current() == '-';
            if (not allow_negative and starts_with_minus) {
                return std::unexpected{ ParseError{ "unexpected negative integer" } };
            }

            if (starts_with_minus) {
                advance();  // consume '-'
                auto integer_result = integer(false);
                if (not integer_result.has_value()) {
                    return std::unexpected{ integer_result.error() };
                }
                return -integer_result.value();
            }

            auto const start_iterator = m_current;
            static constexpr auto is_digit = [](Utf8Char const c) {
                return std::isdigit(static_cast<unsigned char>(c.as_string_view().front()));
            };
            while (not is_at_end_of_input() and is_digit(current())) {
                advance();
            }
            auto const end_iterator = m_current;
            auto const integer_string_view = Utf8StringView{ start_iterator, end_iterator };
            static constexpr auto max_integer_length = std::numeric_limits<i64>::digits10 + 1;
            // for ASCII digits, the number of bytes is the same as the number of digits
            if (integer_string_view.num_bytes() > max_integer_length) {
                return std::unexpected{ ParseError{ "integer out of range" } };
            }
            auto conversion_buffer = c2k::StaticVector<char, max_integer_length>{};
            for (auto const c : integer_string_view) {
                conversion_buffer.push_back(c.as_string_view().front());
            }
            auto result = i64{};
            auto const conversion_result =
                std::from_chars(&*conversion_buffer.cbegin(), &*conversion_buffer.cend(), result);
            if (conversion_result.ec != std::errc{} or conversion_result.ptr != &*conversion_buffer.cend()) {
                return std::unexpected{ ParseError{ "integer out of range" } };
            }
            return result;
        }

        [[nodiscard]] std::expected<ValuePointer, Error> null() {
            static constexpr auto null = std::array{ 'n', 'u', 'l', 'l' };
            if (not try_consume_character_sequence(null)) {
                return std::unexpected{ ParseError{ "expected 'null'" } };
            }
            return std::make_unique<Null>();
        }

        [[nodiscard]] std::expected<ValuePointer, Error> boolean() {
            static constexpr auto true_ = std::array{ 't', 'r', 'u', 'e' };
            if (current().as_string_view().front() == 't') {
                if (try_consume_character_sequence(true_)) {
                    return std::make_unique<Boolean>(true);
                }
                return std::unexpected{ ParseError{ "expected 'true'" } };
            }

            static constexpr auto false_ = std::array{ 'f', 'a', 'l', 's', 'e' };
            if (try_consume_character_sequence(false_)) {
                return std::make_unique<Boolean>(false);
            }
            return std::unexpected{ ParseError{ "expected 'false'" } };
        }

        template<usize length>
        [[nodiscard]] bool try_consume_character_sequence(std::array<char, length> const& sequence) {
            for (auto const c : sequence) {
                if (is_at_end_of_input() or current().as_string_view().front() != c) {
                    return false;
                }
                advance();
            }
            return true;
        }

        void consume_whitespace() {
            static constexpr auto is_whitespace = [](Utf8Char const c) {
                return c == 0x20 or c == 0x0A or c == 0x0D or c == 0x09;
            };

            while (not is_at_end_of_input() and is_whitespace(current())) {
                advance();
            }
        }

        [[nodiscard]] bool is_at_end_of_input() const {
            return m_current == m_input.cend();
        }

        [[nodiscard]] Utf8Char current() const {
            if (is_at_end_of_input()) {
                return Utf8Char{ '\0' };
            }
            return *m_current;
        }

        [[nodiscard]] Utf8Char peek() const {
            if (is_at_end_of_input()) {
                return Utf8Char{ '\0' };
            }
            auto const next = m_current + 1;
            if (next == m_input.cend()) {
                return Utf8Char{ '\0' };
            }
            return *next;
        }

        void advance() {
            if (not is_at_end_of_input()) {
                ++m_current;
            }
        }

        [[nodiscard]] std::expected<std::monostate, Error> consume(char const c) {
            if (current() != c) {
                return std::unexpected{ ParseError{ std::format("expected '{}'", c) } };
            }
            advance();
            return std::monostate{};
        }
    };
}  // namespace c2k::json::detail
