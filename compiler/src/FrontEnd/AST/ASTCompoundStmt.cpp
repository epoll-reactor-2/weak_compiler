#include "FrontEnd/AST/ASTCompoundStmt.hpp"
#include "FrontEnd/AST/ASTVisitor.hpp"

namespace weak {
namespace frontEnd {

ASTCompoundStmt::ASTCompoundStmt(std::vector<std::unique_ptr<ASTNode>> &&stmts,
                                 unsigned TheLineNo, unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Stmts(std::move(stmts)) {}

ASTType ASTCompoundStmt::GetASTType() const { return ASTType::COMPOUND_STMT; }

std::vector<std::unique_ptr<ASTNode>> &&ASTCompoundStmt::GetStmts() {
  return std::move(Stmts);
}

const std::vector<std::unique_ptr<ASTNode>> &ASTCompoundStmt::GetStmts() const {
  return Stmts;
}

} // namespace frontEnd
} // namespace weak