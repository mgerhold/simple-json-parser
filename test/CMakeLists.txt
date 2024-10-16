 CPMAddPackage(
         NAME GTEST
         GITHUB_REPOSITORY google/googletest
         VERSION 1.14.0
         OPTIONS
         "INSTALL_GTEST OFF"
         "BUILD_SHARED_LIBS OFF"
 )

 add_executable(simple_json_parser_tests simple_json_parser_tests.cpp)
 target_link_libraries(simple_json_parser_tests
         PRIVATE
         simple_json_parser_project_options
 )
 target_link_system_libraries(simple_json_parser_tests
         PRIVATE
         gtest_main
         gmock_main
 )

include(GoogleTest)
gtest_discover_tests(simple_json_parser_tests)
