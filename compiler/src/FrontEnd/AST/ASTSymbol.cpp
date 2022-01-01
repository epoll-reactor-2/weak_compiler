#include "FrontEnd/AST/ASTSymbol.hpp"

namespace weak {
namespace frontEnd {

ASTSymbol::ASTSymbol(std::string TheValue, unsigned TheLineNo,
                     unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Value(std::move(TheValue)) {}

ASTType ASTSymbol::GetASTType() const { return ASTType::SYMBOL; }

const std::string &ASTSymbol::GetValue() const { return Value; }

} // namespace frontEnd
} // namespace weak