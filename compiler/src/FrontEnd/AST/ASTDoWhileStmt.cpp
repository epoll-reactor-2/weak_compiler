#include "FrontEnd/AST/ASTDoWhileStmt.hpp"
#include "FrontEnd/AST/ASTVisitor.hpp"

namespace weak {
namespace frontEnd {

weak::frontEnd::ASTDoWhileStmt::ASTDoWhileStmt(
    std::unique_ptr<ASTCompoundStmt> &&TheBody,
    std::unique_ptr<ASTNode> &&TheCondition, unsigned TheLineNo,
    unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Body(std::move(TheBody)),
      Condition(std::move(TheCondition)) {}

ASTType ASTDoWhileStmt::GetASTType() const { return ASTType::DO_WHILE_STMT; }

std::unique_ptr<ASTCompoundStmt> &&ASTDoWhileStmt::GetBody() {
  return std::move(Body);
}

const std::unique_ptr<ASTCompoundStmt> &ASTDoWhileStmt::GetBody() const {
  return Body;
}

std::unique_ptr<ASTNode> &&ASTDoWhileStmt::GetCondition() {
  return std::move(Condition);
}

const std::unique_ptr<ASTNode> &ASTDoWhileStmt::GetCondition() const {
  return Condition;
}

} // namespace frontEnd
} // namespace weak