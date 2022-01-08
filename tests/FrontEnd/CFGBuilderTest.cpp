#include "FrontEnd/Analysis/CFGBuilder.hpp"
#include "FrontEnd/AST/ASTPrettyPrint.hpp"
#include "FrontEnd/Lex/Lexer.hpp"
#include "FrontEnd/Parse/Parser.hpp"
#include "TestHelpers.hpp"

using namespace weak::frontEnd;

static Lexer CreateLexer(std::string_view Input) {
  Lexer Lex(Input.begin(), Input.end());
  return Lex;
}

static void CreateCFG(std::string_view String) {
  auto Tokens = CreateLexer(String).Analyze();
  Parser Parse(&*Tokens.begin(), &*Tokens.end());
  auto AST = Parse.Parse();
  CFGBuilder Builder(std::move(AST));
  auto CFG = Builder.BuildCFG();
}

int main() {
  SECTION(CFGBasic) {
    CreateCFG("void f() {"
              "  if (1) {"
              "    call(2);"
              "  } else {"
              "    call(3);"
              "  }"
              "  call(4);"
              "}");
  }
  SECTION(CFGNestedIf) {
    CreateCFG("void f() {"
              "  if (1) {"
              "    a = 2;"
              "    a = 22;"
              "    a = 222;"
              "    if (3) {"
              "      b = 4;"
              "      if (5) {"
              "        c = 6;"
              "      }"
              "    }"
              "  }"
              "  d = 7;"
              "}");
  }
  SECTION(CFGDeepNestedIfElse) {
    CreateCFG("void f() {"
              "  for (a; b; c) {"
              "    if (1) {"
              "      call(2);"
              "      call(3);"
              "      call(4);"
              "      if (5) {"
              "         call(6);"
              "       if (7) {"
              "         call(8);"
              "       } else {"
              "         call(9);"
              "       }"
              "      }"
              "    } else {"
              "      call(10);"
              "    }"
              "    call(11);"
              "  }"
              "}");
  }
  SECTION(CFGForLoop) {
    CreateCFG("void f() {"
              "  f(0);"
              "  for (a; b; c) {"
              "    for (d; e; f) {"
              "      f(a);"
              "    }"
              "    f(b);"
              "  }"
              "  f(c);"
              "}");
  }
  SECTION(CFGWhileLoop) {
    CreateCFG("void f() {"
              "  while (f(1)) {"
              "    f(2);"
              "  }"
              "  f(3);"
              "}");
  }
  SECTION(CFGDoWhileLoop) {
    CreateCFG("void f() {"
              "  do {"
              "    f(1); f(2);"
              "  } while (f(3));"
              "  f(4);"
              "}");
  }
  SECTION(CFGCompoundLoops) {
    CreateCFG("void f() {"
              "  while (1) {"
              "    do_while_body(0);"
              "  }"
              "  do {"
              "    do_do_while_body(0);"
              "  } while(1);"
              "  for (a; b; c) {"
              "    do_for_body(0);"
              "  }"
              "  do_after(0);"
              "}");
  }
}