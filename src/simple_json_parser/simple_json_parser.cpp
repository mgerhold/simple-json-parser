#include <simple_json_parser/detail/parser.hpp>
#include <simple_json_parser/simple_json_parser.hpp>

namespace c2k::json {

    [[nodiscard]] std::expected<ValuePointer, Error> parse(
        Utf8StringView input,
        tl::optional<std::filesystem::path const&>
    ) {
        auto parser = detail::Parser{ input };
        return parser.parse();
    }
}  // namespace c2k::json
