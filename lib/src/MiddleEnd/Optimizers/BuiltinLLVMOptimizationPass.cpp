/* BuiltinLLVMOptimizationPass.cpp - LLVM IR basic optimization.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "MiddleEnd/Optimizers/Optimizers.h"
#include "Utility/Compiler.h"

WEAK_PRAGMA_PUSH
WEAK_PRAGMA_IGNORE(-Wunused)
WEAK_PRAGMA_IGNORE(-Wunused-parameter)
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
WEAK_PRAGMA_POP

void weak::RunBuiltinLLVMOptimizationPass(llvm::Module &IRModule, WeakOptimizationLevel OptLvl) {
  llvm::legacy::FunctionPassManager FPM(&IRModule);
  llvm::PassManagerBuilder PassBuilder;
  PassBuilder.OptLevel = static_cast<unsigned>(OptLvl);
  PassBuilder.populateFunctionPassManager(FPM);

  for (auto &Func : IRModule.getFunctionList())
    FPM.run(Func);
}