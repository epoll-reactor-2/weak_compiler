/* Diagnostic.hpp - Helper functions, used to emitting errors and warns.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_UTILITY_DIAGNOSTIC_HPP
#define WEAK_COMPILER_UTILITY_DIAGNOSTIC_HPP

#include <ostream>

class Diagnostic;

namespace weak {
namespace frontEnd {
class ASTNode;
} // namespace frontEnd
} // namespace weak

namespace weak {

/// This requires the string as first argument in diagnostic messages.
///
/// \note Such measure was taken to reduce "sstream" header bloat and not much
///       harm compilation time.
struct OstreamRAII {
  ::Diagnostic *DiagImpl;
  ~OstreamRAII() noexcept(false);
  std::ostream &operator<<(const char *);
};

/// Throw an exception with given message.
[[noreturn]] void UnreachablePoint(const char * = "");

/// Print diagnostic message with WARN flag.
OstreamRAII CompileWarning();

/// Print diagnostic message with WARN flag.
OstreamRAII CompileWarning(unsigned LineNo, unsigned ColumnNo);

/// Print diagnostic message with ERROR flag and throw exception.
OstreamRAII CompileError();

/// Print diagnostic message with ERROR flag and throw exception.
OstreamRAII CompileError(unsigned LineNo, unsigned ColumnNo);

/// Print diagnostic message (with position in text) with ERROR flag and
/// throw exception.
///
/// \param Node used to extract line and column number.
weak::OstreamRAII CompileError(const weak::frontEnd::ASTNode *Node);

} // namespace weak

#endif // WEAK_COMPILER_UTILITY_DIAGNOSTIC_HPP