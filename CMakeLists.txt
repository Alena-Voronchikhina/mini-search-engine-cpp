cmake_minimum_required(VERSION 3.20)
project(mini-search-engine-cpp VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MSE_BUILD_TESTS "Build unit/integration tests" ON)
option(MSE_BUILD_BENCH "Build benchmark harness" ON)
option(MSE_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(MSE_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if(MSE_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()
if(MSE_ENABLE_UBSAN)
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined)
endif()

add_library(mse
  src/tokenizer.cpp
  src/stemmer.cpp
  src/index.cpp
  src/intersect.cpp
  src/query_parser.cpp
  src/query_eval.cpp
  src/ranker.cpp
  src/serialize.cpp
)
add_library(mse::mse ALIAS mse)
target_include_directories(mse
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_executable(search src/main.cpp)
target_link_libraries(search PRIVATE mse)

include(GNUInstallDirs)
install(TARGETS search mse
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/mse DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Copy fixture corpus next to build tree for CLI/tests.
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

if(MSE_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.4
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  include(CTest)
  include(Catch)

  add_executable(mse_tests
    tests/test_tokenizer.cpp
    tests/test_stemmer.cpp
    tests/test_intersect.cpp
    tests/test_parser.cpp
    tests/test_eval.cpp
    tests/test_phrase.cpp
    tests/test_ranker.cpp
    tests/test_serialize.cpp
    tests/test_integration.cpp
  )
  target_link_libraries(mse_tests PRIVATE mse Catch2::Catch2WithMain)
  catch_discover_tests(mse_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(MSE_BUILD_BENCH)
  add_executable(mse_bench benchmarks/bench_main.cpp)
  target_link_libraries(mse_bench PRIVATE mse)
endif()
