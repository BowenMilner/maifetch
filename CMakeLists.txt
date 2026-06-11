cmake_minimum_required(VERSION 3.22)

project(maifetch VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)
find_package(PNG)

add_library(maifetch_core
    src/ascii_image.cpp
    src/config.cpp
    src/http.cpp
    src/maitea.cpp
    src/output.cpp
    src/tiny_json.cpp
)

target_include_directories(maifetch_core PUBLIC include)
target_link_libraries(maifetch_core PUBLIC CURL::libcurl)
if (PNG_FOUND)
    target_link_libraries(maifetch_core PUBLIC PNG::PNG)
    target_compile_definitions(maifetch_core PRIVATE MAIFETCH_WITH_PNG=1)
endif()

if (MSVC)
    target_compile_options(maifetch_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(maifetch_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(maifetch src/main.cpp)
target_link_libraries(maifetch PRIVATE maifetch_core)

enable_testing()

add_executable(maifetch_tests
    tests/config_tests.cpp
    tests/output_tests.cpp
    tests/json_tests.cpp
)
target_link_libraries(maifetch_tests PRIVATE maifetch_core)
add_test(NAME maifetch_tests COMMAND maifetch_tests)
