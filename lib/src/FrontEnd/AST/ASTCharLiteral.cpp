/* ASTCharLiteral.cpp - AST node to represent a single character.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTCharLiteral.h"
#include "FrontEnd/AST/ASTVisitor.h"

namespace weak {

ASTCharLiteral::ASTCharLiteral(char Value, unsigned LineNo, unsigned ColumnNo)
    : ASTNode(AST_CHAR_LITERAL, LineNo, ColumnNo), mValue(Value) {}

void ASTCharLiteral::Accept(ASTVisitor *Visitor) { Visitor->Visit(this); }

char ASTCharLiteral::Value() const { return mValue; }

} // namespace weak