/* ScalarExprEmitter.cpp - Generator of operations on numeric data types.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "MiddleEnd/CodeGen/ScalarExprEmitter.hpp"
#include "Utility/Diagnostic.hpp"
#include "llvm/IR/Value.h"

static std::string TypeToString(llvm::Type *T) {
  std::string Type;
  llvm::raw_string_ostream Stream(Type);
  T->print(Stream);
  return Stream.str();
}

namespace weak {

ScalarExprEmitter::ScalarExprEmitter(llvm::LLVMContext &C, llvm::IRBuilder<> &I)
    : IRCtx(C), IRBuilder(I) {}

llvm::Value *ScalarExprEmitter::EmitBinOp(const ASTNode *InformAST, TokenType T,
                                          llvm::Value *L, llvm::Value *R) {
  assert(L->getType() == R->getType());

  if (L->getType()->isIntegerTy())
    return EmitIntegralBinOp(InformAST, T, L, R);

  if (L->getType()->isFloatTy())
    return EmitFloatBinOp(InformAST, T, L, R);

  weak::CompileError(InformAST)
      << "Cannot emit binary instruction, invalid operands: "
      << TypeToString(L->getType()) << " and " << TypeToString(R->getType());
  weak::UnreachablePoint();
}

llvm::Value *ScalarExprEmitter::EmitIntegralBinOp(const ASTNode *InformAST,
                                                  TokenType T, llvm::Value *L,
                                                  llvm::Value *R) {
  switch (T) {
  case TOK_PLUS:
    return IRBuilder.CreateAdd(L, R);
  case TOK_MINUS:
    return IRBuilder.CreateSub(L, R);
  case TOK_STAR:
    return IRBuilder.CreateMul(L, R);
  case TOK_SLASH:
    return IRBuilder.CreateSDiv(L, R);
  case TOK_LE:
    return IRBuilder.CreateICmpSLE(L, R);
  case TOK_LT:
    return IRBuilder.CreateICmpSLT(L, R);
  case TOK_GE:
    return IRBuilder.CreateICmpSGE(L, R);
  case TOK_GT:
    return IRBuilder.CreateICmpSGT(L, R);
  case TOK_EQ:
    return IRBuilder.CreateICmpEQ(L, R);
  case TOK_NEQ:
    return IRBuilder.CreateICmpNE(L, R);
  case TOK_OR:
    return IRBuilder.CreateLogicalOr(L, R);
  case TOK_AND:
    return IRBuilder.CreateLogicalAnd(L, R);
  case TOK_BIT_OR:
    return IRBuilder.CreateOr(L, R);
  case TOK_BIT_AND:
    return IRBuilder.CreateAnd(L, R);
  case TOK_XOR:
    return IRBuilder.CreateXor(L, R);
  case TOK_SHL:
    return IRBuilder.CreateShl(L, R);
  case TOK_SHR:
    return IRBuilder.CreateAShr(L, R);
  default:
    weak::CompileError(InformAST)
        << "Cannot apply `" << weak::TokenToString(T) << "` to integers";
    weak::UnreachablePoint();
  } // switch
}

llvm::Value *ScalarExprEmitter::EmitFloatBinOp(const ASTNode *InformAST,
                                               TokenType T, llvm::Value *L,
                                               llvm::Value *R) {
  switch (T) {
  case TOK_PLUS:
    return IRBuilder.CreateFAdd(L, R);
  case TOK_MINUS:
    return IRBuilder.CreateFSub(L, R);
  case TOK_STAR:
    return IRBuilder.CreateFMul(L, R);
  case TOK_SLASH:
    return IRBuilder.CreateFDiv(L, R);
  case TOK_LE:
    return IRBuilder.CreateFCmpOLE(L, R);
  case TOK_LT:
    return IRBuilder.CreateFCmpOLT(L, R);
  case TOK_GE:
    return IRBuilder.CreateFCmpOGE(L, R);
  case TOK_GT:
    return IRBuilder.CreateFCmpOGT(L, R);
  case TOK_EQ:
    return IRBuilder.CreateFCmpOEQ(L, R);
  case TOK_NEQ:
    return IRBuilder.CreateFCmpONE(L, R);
  case TOK_OR:
    return IRBuilder.CreateLogicalOr(L, R);
  case TOK_AND:
    return IRBuilder.CreateLogicalAnd(L, R);
  default:
    weak::CompileError(InformAST)
        << "Cannot apply `" << TokenToString(T) << "` to floats";
    weak::UnreachablePoint();
  } // switch
}

} // namespace weak