include(${PROJECT_SOURCE_DIR}/cmake/warnings.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake)

# the following function was taken from:
# https://github.com/cpp-best-practices/cmake_template/blob/main/ProjectOptions.cmake
macro(check_sanitizer_support)
    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
        set(supports_ubsan ON)
    else ()
        set(supports_ubsan OFF)
    endif ()

    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(supports_asan OFF)
    else ()
        set(supports_asan ON)
    endif ()
endmacro()

if (PROJECT_IS_TOP_LEVEL)
    option(simple_json_parser_warnings_as_errors "Treat warnings as errors" ON)
    option(simple_json_parser_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" ${supports_ubsan})
    option(simple_json_parser_enable_address_sanitizer "Enable address sanitizer" ${supports_asan})
    option(simple_json_parser_build_tests "Build unit tests" ON)
else ()
    option(simple_json_parser_warnings_as_errors "Treat warnings as errors" OFF)
    option(simple_json_parser_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" OFF)
    option(simple_json_parser_enable_address_sanitizer "Enable address sanitizer" OFF)
    option(simple_json_parser_build_tests "Build unit tests" OFF)
endif ()
option(simple_json_parser_build_shared_libs "Build shared libraries instead of static libraries" ON)
set(BUILD_SHARED_LIBS ${simple_json_parser_build_shared_libs})

add_library(simple_json_parser_warnings INTERFACE)
simple_json_parser_set_warnings(simple_json_parser_warnings ${simple_json_parser_warnings_as_errors})

add_library(simple_json_parser_sanitizers INTERFACE)
simple_json_parser_enable_sanitizers(simple_json_parser_sanitizers ${simple_json_parser_enable_address_sanitizer} ${simple_json_parser_enable_undefined_behavior_sanitizer})

add_library(simple_json_parser_project_options INTERFACE)
target_link_libraries(simple_json_parser_project_options
        INTERFACE simple_json_parser_warnings
        INTERFACE simple_json_parser_sanitizers
)
