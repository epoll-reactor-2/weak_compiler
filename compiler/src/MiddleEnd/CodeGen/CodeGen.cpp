/* CodeGen.cpp - Intermediate code generator.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "MiddleEnd/CodeGen/CodeGen.hpp"
#include "FrontEnd/AST/ASTBinaryOperator.hpp"
#include "FrontEnd/AST/ASTBooleanLiteral.hpp"
#include "FrontEnd/AST/ASTBreakStmt.hpp"
#include "FrontEnd/AST/ASTCompoundStmt.hpp"
#include "FrontEnd/AST/ASTContinueStmt.hpp"
#include "FrontEnd/AST/ASTDoWhileStmt.hpp"
#include "FrontEnd/AST/ASTFloatingPointLiteral.hpp"
#include "FrontEnd/AST/ASTForStmt.hpp"
#include "FrontEnd/AST/ASTFunctionCall.hpp"
#include "FrontEnd/AST/ASTFunctionDecl.hpp"
#include "FrontEnd/AST/ASTIfStmt.hpp"
#include "FrontEnd/AST/ASTIntegerLiteral.hpp"
#include "FrontEnd/AST/ASTNode.hpp"
#include "FrontEnd/AST/ASTReturnStmt.hpp"
#include "FrontEnd/AST/ASTStringLiteral.hpp"
#include "FrontEnd/AST/ASTSymbol.hpp"
#include "FrontEnd/AST/ASTUnaryOperator.hpp"
#include "FrontEnd/AST/ASTVarDecl.hpp"
#include "FrontEnd/AST/ASTWhileStmt.hpp"

using namespace weak::frontEnd;

namespace {

template <class... Ts> struct Overload : Ts... { using Ts::operator()...; };
template <class... Ts> Overload(Ts...) -> Overload<Ts...>;

} // namespace

namespace weak {
namespace middleEnd {

CodeGen::CodeGen(frontEnd::ASTNode *TheRootNode)
    : RootNode(TheRootNode), Emitter(), LastInstruction(0),
      CurrentGotoLabel(0U), Instructions() {}

void CodeGen::CreateCode() {
  RootNode->Accept(this);
  Emitter.Dump();
}

void CodeGen::Visit(const frontEnd::ASTCompoundStmt *Compound) const {
  for (const auto &Stmt : Compound->GetStmts())
    Stmt->Accept(this);
}

void CodeGen::Visit(const frontEnd::ASTFunctionDecl *FunctionDecl) const {
  FunctionDecl->GetBody()->Accept(this);
}

void CodeGen::Visit(const frontEnd::ASTBinaryOperator *Binary) const {
  Binary->GetLHS()->Accept(this);
  auto LHS = LastInstruction;

  Binary->GetRHS()->Accept(this);
  auto RHS = LastInstruction;

  using Ref = InstructionReference;
  // clang-format off
  std::visit(Overload {
    [this, &Binary](const Instruction& L, const Instruction& R) {
      Emitter.Emit(Binary->GetOperation(), Ref(L), Ref(R));
    },
    [this, &Binary](const Instruction& L, auto R) {
      Emitter.Emit(Binary->GetOperation(), Ref(L), R);
    },
    [this, &Binary](auto L, const Instruction& R) {
      Emitter.Emit(Binary->GetOperation(), L, Ref(R));
    },
    [this, &Binary](auto L, auto R) {
      Emitter.Emit(Binary->GetOperation(), L, R);
    }
  }, LHS, RHS);
  // clang-format on

  LastInstruction = std::get<Instruction>(Emitter.GetInstructions().back());
}

void CodeGen::Visit(const frontEnd::ASTIntegerLiteral *Integer) const {
  LastInstruction = Integer->GetValue();
}

void CodeGen::Visit(const frontEnd::ASTVarDecl *VarDecl) const {
  VarDecl->GetDeclareBody()->Accept(this);
}

void CodeGen::Visit(const frontEnd::ASTBooleanLiteral *) const {}
void CodeGen::Visit(const frontEnd::ASTBreakStmt *) const {}
void CodeGen::Visit(const frontEnd::ASTContinueStmt *) const {}
void CodeGen::Visit(const frontEnd::ASTDoWhileStmt *) const {}

void CodeGen::Visit(const frontEnd::ASTFloatingPointLiteral *Float) const {
  LastInstruction = Float->GetValue();
}

void CodeGen::Visit(const frontEnd::ASTForStmt *) const {}
void CodeGen::Visit(const frontEnd::ASTFunctionCall *) const {}

/*!               if !cond then goto EXIT
 *                instr1
 *                instr2
 *  EXIT:         after if instr
 *
 *                if !cond then goto ELSE
 *                instr1
 *                instr2
 *                goto EXIT
 *  ELSE:         else instr1
 *                else instr2
 *  EXIT:         after if instr
 */
void CodeGen::Visit(const frontEnd::ASTIfStmt *If) const {
  unsigned SavedGotoLabel = CurrentGotoLabel++;

  switch (If->GetCondition()->GetASTType()) {
  case ASTType::INTEGER_LITERAL: {
    auto *Int = static_cast<ASTIntegerLiteral *>(If->GetCondition().get());
    Emitter.EmitIf(TokenType::NEQ, Int->GetValue(), 0, SavedGotoLabel + 1);
    Emitter.EmitJump(SavedGotoLabel);
    Emitter.EmitGotoLabel(SavedGotoLabel + 1);
    break;
  }
  case ASTType::BINARY: {
    If->GetCondition()->Accept(this);
    Emitter.EmitJump(SavedGotoLabel);
    Emitter.EmitGotoLabel(SavedGotoLabel + 1);
    break;
  }
  default:
    break;
  }

  If->GetThenBody()->Accept(this);

  Emitter.EmitGotoLabel(SavedGotoLabel);

  if (If->GetElseBody().get()) {
    If->GetElseBody()->Accept(this);
  }
}

void CodeGen::Visit(const frontEnd::ASTReturnStmt *) const {}
void CodeGen::Visit(const frontEnd::ASTStringLiteral *) const {}
void CodeGen::Visit(const frontEnd::ASTSymbol *) const {}

void CodeGen::Visit(const frontEnd::ASTUnaryOperator *Unary) const {
  Unary->GetOperand()->Accept(this);
  using Ref = InstructionReference;

  auto VisitUnary = [this](TokenType Operation, auto &&Instr) {
    // clang-format off
    std::visit(Overload {
      [this, Op = Operation](const Instruction &I) {
        Emitter.Emit(Op, Ref(I), 1);
      },
      [this, Op = Operation](auto I) {
        Emitter.Emit(Op, I, 1);
      }
    }, Instr);
    // clang-format on
  };

  switch (Unary->GetOperation()) {
  case TokenType::INC:
    VisitUnary(TokenType::PLUS, LastInstruction);
    break;
  case TokenType::DEC:
    VisitUnary(TokenType::MINUS, LastInstruction);
    break;
  default:
    break;
  }
}

void CodeGen::Visit(const frontEnd::ASTWhileStmt *) const {}

} // namespace middleEnd
} // namespace weak