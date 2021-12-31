#include "FrontEnd/AST/ASTBinaryOperator.hpp"
#include "FrontEnd/AST/ASTVisitor.hpp"

namespace weak {
namespace frontEnd {

ASTBinaryOperator::ASTBinaryOperator(TokenType TheOperation,
                                     std::unique_ptr<ASTNode> &&TheLHS,
                                     std::unique_ptr<ASTNode> &&TheRHS,
                                     unsigned TheLineNo, unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Operation(TheOperation),
      LHS(std::move(TheLHS)), RHS(std::move(TheRHS)) {}

ASTType ASTBinaryOperator::GetASTType() const { return ASTType::BINARY; }

void ASTBinaryOperator::Accept(
    const std::unique_ptr<ASTVisitor> &Visitor) const {
  Visitor->Visit(this);
}

TokenType ASTBinaryOperator::GetOperation() const { return Operation; }

const std::unique_ptr<ASTNode> &ASTBinaryOperator::GetLHS() const {
  return LHS;
}
const std::unique_ptr<ASTNode> &ASTBinaryOperator::GetRHS() const {
  return RHS;
}

} // namespace frontEnd
} // namespace weak