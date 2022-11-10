/* TypeCheck.cpp - Helper functions to do type checks.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "MiddleEnd/CodeGen/TypeCheck.h"
#include "Utility/Diagnostic.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

static std::string TypeToString(llvm::Type *T) {
  std::string Type;
  llvm::raw_string_ostream Stream(Type);
  T->print(Stream);
  return Stream.str();
}

void weak::AssertSame(ASTNode *InformAST, llvm::Type *L, llvm::Type *R) {
  if (L == R)
    return;

  weak::CompileError(InformAST)
      << "Type mismatch: " << TypeToString(L) << " and " << TypeToString(R);
}

void weak::AssertSame(ASTNode *InformAST, llvm::Value *L, llvm::Value *R) {
  AssertSame(InformAST, L->getType(), R->getType());
}

void weak::AssertNotOutOfRange(ASTNode *InformAST,
                               llvm::AllocaInst *ArrayAlloca,
                               llvm::Value *Index) {
  auto *ConstantArray =
      static_cast<llvm::ArrayType *>(ArrayAlloca->getAllocatedType());
  int64_t ArraySize = ConstantArray->getNumElements();

  if (auto *I = llvm::dyn_cast<llvm::ConstantInt>(Index)) {
    int64_t NumericIndex = I->getSExtValue();
    if (NumericIndex >= ArraySize)
      weak::CompileError(InformAST)
          << "Out of range! Index (which is " << NumericIndex
          << ") >= array size (which is " << ArraySize << ")";
  }
}
