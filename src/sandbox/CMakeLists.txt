add_executable(sandbox main.cpp)
target_link_libraries(sandbox
        PRIVATE
        simple_json_parser_project_options
        simple_json_parser
)
