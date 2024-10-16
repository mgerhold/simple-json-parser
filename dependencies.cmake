include("${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/system_link.cmake")

function(simple_json_parser_setup_dependencies)
    CPMAddPackage(
            NAME LIB2K
            GITHUB_REPOSITORY mgerhold/lib2k
            VERSION 0.0.6
            OPTIONS
            "BUILD_SHARED_LIBS OFF"
    )
    CPMAddPackage(
            NAME TL_OPTIONAL
            GITHUB_REPOSITORY TartanLlama/optional
            VERSION 1.1.0
            OPTIONS
            "OPTIONAL_BUILD_PACKAGE OFF"
            "OPTIONAL_BUILD_TESTS OFF"
            "OPTIONAL_BUILD_PACKAGE_DEB OFF"
            "BUILD_SHARED_LIBS OFF"
    )
endfunction()
