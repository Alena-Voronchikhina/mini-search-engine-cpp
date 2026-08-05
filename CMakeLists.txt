cmake_minimum_required(VERSION 3.20)
project(mini-search-engine-cpp VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MSE_BUILD_TESTS "Build unit/integration tests" ON)
option(MSE_BUILD_BENCH "Build benchmark harness" ON)
option(MSE_BUILD_FUZZ "Build parser/tokenizer fuzz smoke binary" ON)
option(MSE_FUZZER_LIBFUZZER "Link fuzz target with libFuzzer (Clang)" OFF)
option(MSE_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(MSE_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(MSE_ENABLE_CLANG_TIDY "Run clang-tidy while compiling" OFF)

if(MSE_ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()
if(MSE_ENABLE_UBSAN)
  add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined)
endif()

if(MSE_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
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
  src/compress.cpp
  src/build.cpp
)
add_library(mse::mse ALIAS mse)
target_include_directories(mse
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
find_package(Threads REQUIRED)
target_link_libraries(mse PUBLIC Threads::Threads)

add_executable(search src/main.cpp)
target_link_libraries(search PRIVATE mse)

include(GNUInstallDirs)
install(TARGETS search mse
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/mse DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

if(MSE_BUILD_TESTS)
  # Prefer vendored Catch2 (offline/CI-stable); fall back to FetchContent.
  set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
  if(EXISTS "${CMAKE_SOURCE_DIR}/third_party/Catch2/CMakeLists.txt")
    add_subdirectory(third_party/Catch2 EXCLUDE_FROM_ALL)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/third_party/Catch2/extras")
  else()
    include(FetchContent)
    FetchContent_Declare(
      Catch2
      URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.4.tar.gz
      URL_HASH SHA256=b7754b711242c167d8f60b890695347f90a1ebc95949a045385114165d606dbb
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
  endif()
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
    tests/test_compress.cpp
    tests/test_build.cpp
  )
  target_link_libraries(mse_tests PRIVATE mse Catch2::Catch2WithMain)
  catch_discover_tests(mse_tests WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(MSE_BUILD_BENCH)
  add_executable(mse_bench benchmarks/bench_main.cpp)
  target_link_libraries(mse_bench PRIVATE mse)
endif()

if(MSE_BUILD_FUZZ)
  add_executable(mse_fuzz fuzz/fuzz_parser.cpp)
  target_link_libraries(mse_fuzz PRIVATE mse)
  if(MSE_FUZZER_LIBFUZZER)
    target_compile_options(mse_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
    target_link_options(mse_fuzz PRIVATE -fsanitize=fuzzer,address,undefined)
  else()
    target_compile_definitions(mse_fuzz PRIVATE MSE_FUZZER_STANDALONE=1)
  endif()
endif()
