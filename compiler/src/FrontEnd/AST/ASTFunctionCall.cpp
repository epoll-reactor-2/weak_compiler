#include "FrontEnd/AST/ASTFunctionCall.hpp"

namespace weak {
namespace frontEnd {

ASTFunctionCall::ASTFunctionCall(
    std::string &&TheName, std::vector<std::unique_ptr<ASTNode>> &&TheArguments,
    unsigned TheLineNo, unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Name(std::move(TheName)),
      Arguments(std::move(TheArguments)) {}

ASTType ASTFunctionCall::GetASTType() const { return ASTType::FUNCTION_CALL; }

const std::string &ASTFunctionCall::GetName() const { return Name; }

const std::vector<std::unique_ptr<ASTNode>> &
ASTFunctionCall::GetArguments() const {
  return Arguments;
}

} // namespace frontEnd
} // namespace weak