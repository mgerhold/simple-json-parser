#include <print>
#include <simple_json_parser/simple_json_parser.hpp>

template<typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

int main() {
    using namespace c2k::Utf8Literals;
    using namespace c2k::json;
    /*auto const object = Object{
            std::pair{ "test1"_utf8, std::make_unique<String>("hello1") },
            std::pair{ "test2"_utf8, std::make_unique<Number>(3.14) },
            std::pair{ "test3"_utf8, std::make_unique<Boolean>(true) },
            std::pair{ "test4"_utf8, std::make_unique<Null>() },
            std::pair{ "test_a"_utf8, std::make_unique<Array>() },
            std::pair{ "test_o"_utf8, std::make_unique<Object>() },
            std::pair{
                "test5"_utf8,
                std::make_unique<Array>(
                    std::make_unique<String>("hello 🦀!"),
                    std::make_unique<Number>(42),
                    std::make_unique<Object>(
                        std::pair{ "test1"_utf8, std::make_unique<String>("hello1") },
                        std::pair{ "test2"_utf8, std::make_unique<Number>(42) },
                        std::pair{ "test3"_utf8, std::make_unique<Boolean>(false) },
                        std::pair{ "test4"_utf8, std::make_unique<Null>() },
                        std::pair{ "test_a"_utf8, std::make_unique<Array>() },
                        std::pair{ "test_o"_utf8, std::make_unique<Object>() }
                    )
                )
            },
        };
    std::println("{}", object.pretty_print().c_str());*/
    auto const input = R"(
[
    true,
    -42,
    null,
    0,
    -0,
    -17,
    3,
    42,
    100,
    99999999999999,
    3.14,
    -3.14,
    0.0,
    2.1,
    "test",
    "Hello, world!",
    "This is a unicode string presenting ferris: 🦀",
    "This\tstring\\includes\/a\bnewline\f [f]\nbetween[windows→]\rits \"words\" [quoted]",
    "test",
    [
        true,
        false,
        null
    ],
    {
        "persons": [
            {
                "name": "Claus Kleber",
                "age": 65
            },
            {
                "name": "Gundula Gause",
                "age": 60
            }
        ]
    },
    {
        "\ud83e\udd80": "Rust",
        "\ud83d\udc00": "C++"
    }
]
)"_utf8;
    if (auto const json = parse(input); json.has_value()) {
        std::println("{}", (*json)->pretty_print().c_str());
    } else {
        std::println(std::cerr, "Error: {}", std::visit([](auto const& error) { return error.message; }, json.error()));
    }
}
