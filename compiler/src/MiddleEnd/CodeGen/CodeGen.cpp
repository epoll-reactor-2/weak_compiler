/* CodeGen.cpp - LLVM IR generator.
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
#include "FrontEnd/AST/ASTReturnStmt.hpp"
#include "FrontEnd/AST/ASTStringLiteral.hpp"
#include "FrontEnd/AST/ASTSymbol.hpp"
#include "FrontEnd/AST/ASTUnaryOperator.hpp"
#include "FrontEnd/AST/ASTVarDecl.hpp"
#include "FrontEnd/AST/ASTWhileStmt.hpp"
#include "MiddleEnd/CodeGen/TypeResolver.hpp"
#include "Utility/Diagnostic.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace weak {
namespace middleEnd {

CodeGen::CodeGen(frontEnd::ASTNode *TheRoot)
    : Root(TheRoot), LastEmitted(nullptr), LLVMCtx(),
      LLVMModule("LLVM Module", LLVMCtx), CodeBuilder(LLVMCtx),
      IsReturnValue(false) {}

void CodeGen::CreateCode() {
  Root->Accept(this);
  if (LastEmitted)
    LastEmitted->print(llvm::errs());
}

void CodeGen::Visit(const frontEnd::ASTCompoundStmt *Stmts) const {
  for (const auto &Stmt : Stmts->GetStmts()) {
    Stmt->Accept(this);
  }
}

void CodeGen::Visit(const frontEnd::ASTIntegerLiteral *Stmt) const {
  llvm::APInt Int(
      /*numBits=*/32,
      /*val=*/Stmt->GetValue(),
      /*isSigned=*/false);
  LastEmitted = llvm::ConstantInt::get(LLVMCtx, Int);
}

void CodeGen::Visit(const frontEnd::ASTSymbol *Stmt) const {
  llvm::Value *V = VariablesMapping[Stmt->GetName()];
  if (!V) {
    weak::CompileError() << "Unknown variable name: " << Stmt->GetName();
    return;
  }
  LastEmitted = V;
}

void CodeGen::Visit(const frontEnd::ASTBinaryOperator *Stmt) const {
  /// \todo: Make type checking, e.g. decide how to handle
  ///        expressions such as 1 + 2.0.
  Stmt->GetLHS()->Accept(this);
  llvm::Value *L = LastEmitted;
  Stmt->GetRHS()->Accept(this);
  llvm::Value *R = LastEmitted;

  if (!L || !R) {
    return;
  }

  switch (Stmt->GetOperation()) {
  case frontEnd::TokenType::PLUS:
    LastEmitted = CodeBuilder.CreateAdd(L, R);
    break;
  case frontEnd::TokenType::MINUS:
    LastEmitted = CodeBuilder.CreateSub(L, R);
    break;
  case frontEnd::TokenType::STAR:
    LastEmitted = CodeBuilder.CreateMul(L, R);
    break;
  case frontEnd::TokenType::SLASH:
    LastEmitted = CodeBuilder.CreateSDiv(L, R);
    break;
  case frontEnd::TokenType::LT:
    LastEmitted = CodeBuilder.CreateSDiv(L, R);
    LastEmitted =
        CodeBuilder.CreateUIToFP(LastEmitted, llvm::Type::getDoubleTy(LLVMCtx));
    break;
  default:
    LastEmitted = nullptr;
    weak::CompileError() << "Invalid binary operator";
    break;
  }
}

void CodeGen::Visit(const frontEnd::ASTVarDecl *Decl) const {
  Decl->GetDeclareBody()->Accept(this);
  VariablesMapping.emplace(Decl->GetSymbolName(), LastEmitted);
}

class FunctionBuilder {
public:
  FunctionBuilder(llvm::LLVMContext &TheCtx, llvm::Module &TheModule,
                  const frontEnd::ASTFunctionDecl *TheDecl)
      : Ctx(TheCtx), Module(TheModule), Decl(TheDecl) {}

  llvm::Function *Build() {
    llvm::FunctionType *Signature = CreateSignature();
    llvm::Function *Func = llvm::Function::Create(
        Signature, llvm::Function::ExternalLinkage, Decl->GetName(), &Module);

    unsigned Idx{0U};
    for (llvm::Argument &Arg : Func->args())
      Arg.setName(ExtractSymbol(Decl->GetArguments()[Idx++].get()));

    return Func;
  }

private:
  llvm::FunctionType *CreateSignature() {
    middleEnd::TypeResolver TypeResolver(Ctx);
    llvm::SmallVector<llvm::Type *, 16> ArgTypes;

    for (const auto &Arg : Decl->GetArguments())
      ArgTypes.push_back(TypeResolver.ResolveFunctionParam(Arg.get()));

    llvm::FunctionType *Signature = llvm::FunctionType::get(
        // Return type.
        TypeResolver.ResolveReturnType(Decl->GetReturnType()),
        // Arguments.
        ArgTypes,
        // Variadic parameters?
        false);

    return Signature;
  }

  static const std::string &ExtractSymbol(const frontEnd::ASTNode *Node) {
    const auto *VarDecl = static_cast<const frontEnd::ASTVarDecl *>(Node);
    return VarDecl->GetSymbolName();
  }

  llvm::LLVMContext &Ctx;
  llvm::Module &Module;
  const frontEnd::ASTFunctionDecl *Decl;
};

void CodeGen::Visit(const frontEnd::ASTFunctionDecl *Decl) const {
  FunctionBuilder FunctionBuilder(LLVMCtx, LLVMModule, Decl);

  llvm::Function *Func = FunctionBuilder.Build();
  llvm::BasicBlock *CFGBlock = llvm::BasicBlock::Create(LLVMCtx, "entry", Func);
  CodeBuilder.SetInsertPoint(CFGBlock);

  VariablesMapping.clear();
  for (auto &Arg : Func->args())
    VariablesMapping.emplace(Arg.getName(), &Arg);

  Decl->GetBody()->Accept(this);

  if (IsReturnValue) {
    llvm::verifyFunction(*Func);
    LastEmitted = Func;
    IsReturnValue = false;
  } else {
    Func->eraseFromParent();
    LastEmitted = nullptr;
  }
}

void CodeGen::Visit(const frontEnd::ASTReturnStmt *Stmt) const {
  Stmt->GetOperand()->Accept(this);
  CodeBuilder.CreateRet(LastEmitted);
  IsReturnValue = true;
}

void CodeGen::Visit(const frontEnd::ASTFunctionCall *Stmt) const {
  llvm::Function *Callee = LLVMModule.getFunction(Stmt->GetName());
  if (!Callee) {
    weak::CompileError() << "Unknown function: " << Stmt->GetName();
    return;
  }

  const auto &FunArgs = Stmt->GetArguments();

  if (Callee->arg_size() != FunArgs.size()) {
    weak::CompileError() << "Arguments size mismatch (" << Callee->arg_size()
                         << " vs " << FunArgs.size() << ")";
    return;
  }

  llvm::SmallVector<llvm::Value *, 16> Args;
  for (size_t I = 0; I < FunArgs.size(); ++I) {
    FunArgs.at(I)->Accept(this);
    Args.push_back(LastEmitted);
    if (!Args.back())
      return;
  }

  LastEmitted = CodeBuilder.CreateCall(Callee, Args);
}

} // namespace middleEnd
} // namespace weak
