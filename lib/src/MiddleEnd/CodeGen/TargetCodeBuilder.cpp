/* TargetCodeBuilder.cpp - Builder of executable code from LLVM IR.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "MiddleEnd/CodeGen/TargetCodeBuilder.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"

static void InitializeLLVMTargets() {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
}

static const llvm::Target *GetTarget(const std::string &Triple) {
  std::string Error;
  auto *Target = llvm::TargetRegistry::lookupTarget(Triple, Error);

  if (!Target) {
    llvm::errs() << Error;
    exit(-1);
  }

  return Target;
}

/// This is a stub to somehow achieve executable file.
static void Link(std::string_view Filename, std::string_view OutObjectPath) {
  std::string CompileCmd;
  CompileCmd += "clang++ ";
  CompileCmd += Filename;
  CompileCmd += " -o ";
  CompileCmd += OutObjectPath;
  system(CompileCmd.c_str());
}

namespace weak {

TargetCodeBuilder::TargetCodeBuilder(llvm::Module &Module,
                                     std::string_view ObjFilePath)
    : mIRModule(Module), mObjFilePath(ObjFilePath) {}

void TargetCodeBuilder::Build() {
  InitializeLLVMTargets();

  auto Triple = llvm::sys::getDefaultTargetTriple();
  const llvm::Target *Target = GetTarget(Triple);

  llvm::TargetOptions Opts;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto *TheTargetMachine = Target->createTargetMachine(
      Triple, /*CPU=*/"generic", /*Features=*/"", Opts, RM);

  mIRModule.setDataLayout(TheTargetMachine->createDataLayout());

  auto Filename = std::string(mObjFilePath) + ".o";
  std::error_code Errc;
  llvm::raw_fd_ostream Dest(Filename, Errc, llvm::sys::fs::OF_None);

  if (Errc) {
    llvm::errs() << "Could not open file: " << Errc.message();
    exit(-1);
  }

  llvm::legacy::PassManager Pass;
  auto FileType = llvm::CGFT_ObjectFile;

  if (TheTargetMachine->addPassesToEmitFile(
          Pass, Dest, /*raw_pwrite_stream=*/nullptr, FileType)) {
    llvm::errs() << "TheTargetMachine can't emit a file of this type";
    exit(-1);
  }

  Pass.run(mIRModule);
  Dest.flush();

  Link(Filename, mObjFilePath);
}

} // namespace weak