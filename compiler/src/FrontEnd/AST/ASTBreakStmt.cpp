#include "FrontEnd/AST/ASTBreakStmt.hpp"

namespace weak {
namespace frontEnd {

ASTBreakStmt::ASTBreakStmt(unsigned TheLineNo, unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo) {}

ASTType ASTBreakStmt::GetASTType() const { return ASTType::BREAK_STMT; }

} // namespace frontEnd
} // namespace weak