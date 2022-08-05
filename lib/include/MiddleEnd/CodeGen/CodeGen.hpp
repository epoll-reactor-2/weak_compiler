/* CodeGen.hpp - LLVM IR generator.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_MIDDLE_END_CODEGEN_HPP
#define WEAK_COMPILER_MIDDLE_END_CODEGEN_HPP

#include "FrontEnd/AST/ASTVisitor.hpp"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>

namespace weak {
namespace middleEnd {

class CodeGen : private frontEnd::ASTVisitor {
public:
  CodeGen(frontEnd::ASTNode *TheRoot);

  /// \todo: Move object file formation somewhere else.
  void CreateCode(std::string_view ObjectFilePath);

  std::string ToString() const;

private:
  // Literals.
  void Visit(const frontEnd::ASTBooleanLiteral *) override;
  void Visit(const frontEnd::ASTIntegerLiteral *) override;
  void Visit(const frontEnd::ASTFloatingPointLiteral *) override;
  void Visit(const frontEnd::ASTStringLiteral *) override;

  // Operators.
  void Visit(const frontEnd::ASTBinaryOperator *) override;
  void Visit(const frontEnd::ASTUnaryOperator *) override;

  // Inside-loop statements.
  void Visit(const frontEnd::ASTBreakStmt *) override {}
  void Visit(const frontEnd::ASTContinueStmt *) override {}

  // Loop statements.
  void Visit(const frontEnd::ASTForStmt *) override;
  void Visit(const frontEnd::ASTWhileStmt *) override;
  void Visit(const frontEnd::ASTDoWhileStmt *) override;

  // Condition statements.
  void Visit(const frontEnd::ASTIfStmt *) override;

  // Function statements.
  void Visit(const frontEnd::ASTFunctionDecl *) override;
  void Visit(const frontEnd::ASTFunctionCall *) override;
  void Visit(const frontEnd::ASTFunctionPrototype *) override;

  // The rest.
  void Visit(const frontEnd::ASTSymbol *) override;
  void Visit(const frontEnd::ASTCompoundStmt *) override;
  void Visit(const frontEnd::ASTReturnStmt *) override;
  void Visit(const frontEnd::ASTVarDecl *) override;

  frontEnd::ASTNode *Root;
  llvm::Value *LastEmitted;
  llvm::LLVMContext LLVMCtx;
  llvm::Module LLVMModule;
  llvm::IRBuilder<> CodeBuilder;
  std::map<std::string, llvm::AllocaInst *> VariablesMapping;
  bool IsReturnValue;
};

} // namespace middleEnd
} // namespace weak

#endif // WEAK_COMPILER_MIDDLE_END_CODEGEN_HPP