/* CodeGen.hpp - Intermediate code generator.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_MIDDLE_END_CODE_GEN_HPP
#define WEAK_COMPILER_MIDDLE_END_CODE_GEN_HPP

#include "FrontEnd/AST/ASTVisitor.hpp"
#include "MiddleEnd/CodeGen/CodeEmitter.hpp"
#include "MiddleEnd/IR/Instruction.hpp"
#include "MiddleEnd/Symbols/Storage.hpp"
#include <list>
#include <map>

namespace weak {
namespace middleEnd {

class CodeGen : private frontEnd::ASTVisitor {
public:
  using AnyInstruction = std::variant<
      /* If statement produces another instruction. */ Instruction,
      /* If statement produces unary instruction. */ UnaryInstruction,
      /* If statement produces reference. */ InstructionReference,
      /* If statement produces digit. */ signed,
      /* If statement produces float. */ double,
      /* If statement produces boolean. */ bool>;

  CodeGen(Storage *TheVariablePool, frontEnd::ASTNode *TheRootNode);

  const std::list<weak::middleEnd::AnyInstruction> &CreateCode();

private:
  void Visit(const frontEnd::ASTBinaryOperator *) const override;
  void Visit(const frontEnd::ASTBooleanLiteral *) const override;
  void Visit(const frontEnd::ASTBreakStmt *) const override;
  void Visit(const frontEnd::ASTCompoundStmt *) const override;
  void Visit(const frontEnd::ASTContinueStmt *) const override;
  void Visit(const frontEnd::ASTDoWhileStmt *) const override;
  void Visit(const frontEnd::ASTFloatingPointLiteral *) const override;
  void Visit(const frontEnd::ASTForStmt *) const override;
  void Visit(const frontEnd::ASTFunctionDecl *) const override;
  void Visit(const frontEnd::ASTFunctionCall *) const override;
  void Visit(const frontEnd::ASTIfStmt *) const override;
  void Visit(const frontEnd::ASTIntegerLiteral *) const override;
  void Visit(const frontEnd::ASTReturnStmt *) const override;
  void Visit(const frontEnd::ASTStringLiteral *) const override;
  void Visit(const frontEnd::ASTSymbol *) const override;
  void Visit(const frontEnd::ASTUnaryOperator *) const override;
  void Visit(const frontEnd::ASTVarDecl *) const override;
  void Visit(const frontEnd::ASTWhileStmt *) const override;

  template <typename DataType>
  IfInstruction *CreateConditionalInstruction(const frontEnd::ASTNode *) const;

  void EmitAssignment(const frontEnd::ASTBinaryOperator *) const;

  IfInstruction *EmitCondition(const frontEnd::ASTNode *) const;

  frontEnd::ASTNode *RootNode;

  mutable CodeEmitter Emitter;

  mutable AnyInstruction LastInstruction;

  mutable unsigned CurrentGotoLabel;

  mutable Storage *VariablePool;
};

} // namespace middleEnd
} // namespace weak

#endif // WEAK_COMPILER_MIDDLE_END_CODE_GEN_HPP
