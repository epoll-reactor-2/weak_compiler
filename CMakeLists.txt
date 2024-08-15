cmake_minimum_required(VERSION 3.9)

project(
  WeakCompiler
  DESCRIPTION "The compiler for unnamed programming language"
  HOMEPAGE_URL "https://github.com/epoll-reactor/weak_compiler"
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

string(
  COMPARE EQUAL
  "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
  IS_TOP_PROJECT
)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake from ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD} support core irreader)

add_subdirectory(lib)

if (IS_TOP_PROJECT)
  enable_testing()
  add_subdirectory(tests)
  add_subdirectory(compiler)
endif()