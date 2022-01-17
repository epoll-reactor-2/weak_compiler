/* CodeEmitter.hpp - Helper class used to form IR.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_MIDDLE_END_CODE_EMITTER_HPP
#define WEAK_COMPILER_MIDDLE_END_CODE_EMITTER_HPP

#include "MiddleEnd/IR/Instruction.hpp"
#include "MiddleEnd/IR/Operations.hpp"
#include "MiddleEnd/IR/Registers.hpp"
#include <list>

namespace weak {
namespace middleEnd {

/// This is used to simplify the formation of program represented in IR.
class CodeEmitter {
public:
  CodeEmitter();

  const Instruction *Emit(frontEnd::TokenType, const Instruction::AnyOperand &,
                          const Instruction::AnyOperand &);
  const Instruction *Emit(const Instruction &);

  const UnaryInstruction *Emit(const UnaryInstruction::AnyOperand &);

  IfInstruction *EmitIf(frontEnd::TokenType Operation,
                        const Instruction::AnyOperand &Left,
                        const Instruction::AnyOperand &Right,
                        unsigned GotoLabel);
  IfInstruction *EmitIf(const Instruction &Instr, unsigned GotoLabel);

  const GotoLabel *EmitGotoLabel(unsigned Label);
  const Jump *EmitJump(unsigned ToLabel);

  void RemoveLast();

  void Dump();

  const std::list<AnyInstruction> &GetInstructions() const;

private:
  std::list<AnyInstruction> Instructions;
  unsigned CurrentLabel;
};

} // namespace middleEnd
} // namespace weak

#endif // WEAK_COMPILER_MIDDLE_END_CODE_EMITTER_HPP