cmake_minimum_required(VERSION 3.16)
project(mini-search-engine-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(search
  src/main.cpp
  src/Indexer.cpp
)
target_include_directories(search PRIVATE include)

# Sample corpus lives next to the binary's working directory in CI/local runs.
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

enable_testing()
add_test(NAME and_query_cats COMMAND search cats)
add_test(NAME and_query_cats_milk COMMAND search cats milk)
add_test(NAME and_query_no_match COMMAND search unicorn)
set_tests_properties(and_query_cats and_query_cats_milk PROPERTIES
  PASS_REGULAR_EXPRESSION "data/doc"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
set_tests_properties(and_query_no_match PROPERTIES
  PASS_REGULAR_EXPRESSION "No results"
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
