#ifndef COMPILER_TEST_HELPERS_HPP
#define COMPILER_TEST_HELPERS_HPP

#include <iostream>

#define TEST_CASE(stmt)                                                        \
  if (!(stmt)) {                                                               \
    std::cout << "ERROR: test case " << #stmt << " failed\n";                  \
    return EXIT_FAILURE;                                                       \
  }

#define SECTION(string)                                                        \
  (std::cout << "testing section " << #string << std::endl);

#endif // COMPILER_TEST_HELPERS_HPP
