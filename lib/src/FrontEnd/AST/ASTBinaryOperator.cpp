/* ASTBinaryOperator.cpp - AST node to represent a binary operator.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTBinaryOperator.hpp"
#include "FrontEnd/AST/ASTVisitor.hpp"

namespace weak {

ASTBinaryOperator::ASTBinaryOperator(TokenType TheOperation, ASTNode *TheLHS,
                                     ASTNode *TheRHS, unsigned TheLineNo,
                                     unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), Operation(TheOperation), LHS(TheLHS),
      RHS(TheRHS) {}

ASTBinaryOperator::~ASTBinaryOperator() {
  delete LHS;
  delete RHS;
}

ASTType ASTBinaryOperator::GetASTType() const { return AST_BINARY; }

void ASTBinaryOperator::Accept(ASTVisitor *Visitor) { Visitor->Visit(this); }

TokenType ASTBinaryOperator::GetOperation() const { return Operation; }

ASTNode *ASTBinaryOperator::GetLHS() const { return LHS; }
ASTNode *ASTBinaryOperator::GetRHS() const { return RHS; }

} // namespace weak