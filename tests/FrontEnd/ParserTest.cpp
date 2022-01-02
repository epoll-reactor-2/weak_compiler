#include "FrontEnd/Lex/Lexer.hpp"
#include "FrontEnd/AST/ASTPrettyPrint.hpp"
#include "FrontEnd/Parse/Parser.hpp"

using namespace weak::frontEnd;

static Lexer CreateLexer(std::string_view Input) {
  Lexer Lex(Input.begin(), Input.end());
  return Lex;
}

static void PrintAST(std::string_view String) {
  auto Tokens = CreateLexer(String).Analyze();
  Parser Parse(&*Tokens.begin(), &*Tokens.end());
  ASTPrettyPrint(Parse.Parse());
}

int main() {
  PrintAST("string do_some_work() {\n"
           "  return 1 + 2 + 3 + 4 + 5;\n"
           "}\n"
           "void main(string symbol, int integer) {\n"
           "  for (a = 0; a < 10; ++a) { a << b + c = x <= e == f >= g; }\n"
           "  while (a << b + c = x) {\n"
           "    while (0) {\n"
           "      a << b + c = a << b + c = x <= e == f >= g <= e == f >= g;\n"
           "      continue;\n"
           "    }\n"
           "    break;\n"
           "  }\n"
           "  do {\n"
           "    a + b * c;\n"
           "  } while (1);\n"
           "  if (1) { a; } else { b; }\n"
           "  int v = a << b + c + ++a;\n"
           "  return;\n"
           "}\n");
}