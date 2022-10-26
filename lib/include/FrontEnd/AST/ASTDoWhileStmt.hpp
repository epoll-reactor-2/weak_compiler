/* ASTDoWhileStmt.hpp - AST node to represent a do-while statement.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_FRONTEND_AST_AST_DO_WHILE_STMT_HPP
#define WEAK_COMPILER_FRONTEND_AST_AST_DO_WHILE_STMT_HPP

#include "FrontEnd/AST/ASTCompoundStmt.hpp"
#include "FrontEnd/AST/ASTNode.hpp"

namespace weak {

class ASTDoWhileStmt : public ASTNode {
public:
  ASTDoWhileStmt(ASTCompoundStmt *TheBody, ASTNode *TheCondition,
                 unsigned TheLineNo = 0U, unsigned TheColumnNo = 0U);

  ~ASTDoWhileStmt();

  void Accept(ASTVisitor *) override;

  ASTCompoundStmt *GetBody() const;
  ASTNode *GetCondition() const;

private:
  ASTCompoundStmt *Body;
  ASTNode *Condition;
};

} // namespace weak

#endif // WEAK_COMPILER_FRONTEND_AST_AST_DO_WHILE_STMT_HPP