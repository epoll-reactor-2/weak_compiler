/* ASTIfStmt.cpp - AST node to represent a if or if-else statements.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTIfStmt.h"
#include "FrontEnd/AST/ASTVisitor.h"

namespace weak {

ASTIfStmt::ASTIfStmt(ASTNode *TheCondition, ASTCompoundStmt *TheThenBody,
                     ASTCompoundStmt *TheElseBody, unsigned TheLineNo,
                     unsigned TheColumnNo)
    : ASTNode(AST_IF_STMT, TheLineNo, TheColumnNo), Condition(TheCondition),
      ThenBody(TheThenBody), ElseBody(TheElseBody) {}

ASTIfStmt::~ASTIfStmt() {
  delete Condition;
  delete ThenBody;
  delete ElseBody;
}

void ASTIfStmt::Accept(ASTVisitor *Visitor) { Visitor->Visit(this); }

ASTNode *ASTIfStmt::GetCondition() const { return Condition; }

ASTCompoundStmt *ASTIfStmt::GetThenBody() const { return ThenBody; }

ASTCompoundStmt *ASTIfStmt::GetElseBody() const { return ElseBody; }

} // namespace weak