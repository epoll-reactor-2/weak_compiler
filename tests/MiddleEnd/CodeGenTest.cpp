#include "FrontEnd/Lex/Lexer.hpp"
#include "FrontEnd/Parse/Parser.hpp"
#include "MiddleEnd/CodeGen/CodeGen.hpp"
#include "MiddleEnd/CodeGen/TargetCodeBuilder.hpp"
#include "TestHelpers.hpp"
#include <fstream>
#include <filesystem>

namespace fe = weak::frontEnd;
namespace me = weak::middleEnd;

void RunFromFile(std::string_view Path) {
  llvm::outs() << "Testing file " << Path << "...\n";
  std::ifstream File(Path.data());
  std::string Program(
    (std::istreambuf_iterator<char>(File)),
    (std::istreambuf_iterator<char>()));
  fe::Lexer Lex(&*Program.begin(), &*Program.end());
  auto Tokens = Lex.Analyze();
  fe::Parser Parser(&*Tokens.begin(), &*Tokens.end());
  auto AST = Parser.Parse();

  me::CodeGen CodeGen(AST.get());

  std::string TargetPath(Path.substr(Path.find_last_of('/') + 1));
  TargetPath = TargetPath.substr(0, TargetPath.find_first_of('.'));

  using namespace std::string_view_literals;
  if (TargetPath.substr(0, "ExpectError"sv.size()) == "ExpectError") {
    if (Program.substr(0, 3) != "// ") {
      llvm::errs() << "Expected comment with error message";
      exit(-1);
    }

    std::string ExpectedErrorMsg = Program.substr(
        "// "sv.length(), Program.find_first_of('\n') - "// "sv.length());

    try {
      CodeGen.CreateCode();
      llvm::errs() << "Expected error";
      exit(-1);
    } catch (std::exception &Error) {
      if (std::string(Error.what()) != ExpectedErrorMsg) {
        llvm::errs() << "Errors mismatch:\n\t" << Error.what()
          << "\ngot, but\n\t" << ExpectedErrorMsg << "\nexpected";
        exit(-1);
      }
      llvm::outs() << "Caught expected error: " << Error.what() << '\n';
    }
  } else {
    CodeGen.CreateCode();
    llvm::outs() << CodeGen.ToString();
    weak::middleEnd::TargetCodeBuilder TargetCodeBuilder(CodeGen.GetModule(), TargetPath);
    TargetCodeBuilder.Build();
  }
}

int main() {
  auto Directory = std::filesystem::directory_iterator(
    std::filesystem::current_path().concat("/CodeGen"));
  for (const auto &File : Directory)
    if (File.path().extension() == ".wl")
      RunFromFile(File.path().native());
}