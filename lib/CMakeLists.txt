# Also add header fiels for conveniencein QtCreator.
file (GLOB_RECURSE SOURCES *.cpp *.h)
add_library(WeakCompiler SHARED "${SOURCES}")

target_link_libraries(WeakCompiler PRIVATE ${llvm_libs})
# target_compile_options(WeakCompiler PRIVATE -Wall -Wextra -Wpedantic -fPIC -flto -O3)
target_compile_options(WeakCompiler PRIVATE -Wall -Wextra -Wpedantic -O0 -ggdb)

if (WEAK_COMPILER_SANITIZE)
  message(STATUS "Building the compiler library with sanitizer flags")
  set(SanFlags "\
    -fsanitize=address -fno-omit-frame-pointer\
    -fsanitize=undefined -fno-sanitize-recover=all\
    -fsanitize-address-use-after-scope"
  )
  message(STATUS "Sanitizer flags: ${SanFlags}")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    string(APPEND SanFlags " -fsanitize=cfi -fvisibility=default -flto")
  endif()

  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    string(APPEND SanFlags " -fanalyzer")
  endif()

  add_compile_options(SanFlags)
  link_libraries(SanFlags)
endif()

include_directories(include)
# include_directories(Compiler PUBLIC include)