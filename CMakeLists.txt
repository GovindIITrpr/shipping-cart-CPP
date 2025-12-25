cmake_minimum_required(VERSION 3.10)

project(dragon_shopping_cart)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(libpqxx CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(dragon_shopping_cart ${SOURCES})

target_link_libraries(dragon_shopping_cart PRIVATE nlohmann_json::nlohmann_json libpqxx::pqxx)