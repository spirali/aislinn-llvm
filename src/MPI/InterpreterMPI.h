//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_INTERPRETERMPI
#define AISLINN_INTERPRETERMPI

#include "ProgramState.h"
#include "../Interpreter/Interpreter.h"

namespace aislinn {

class ProgramState;

class InterpreterMPI : public Interpreter
{
  ProgramState *PState;

  public:
  InterpreterMPI(llvm::Module *M);

  void setProgramState(ProgramState *PS) {
    PState = PS;
  }

  void setProgramState(ProgramState *PS, ProcessState *S) {
    setProgramState(PS, PS->getRank(S));
  }

  ProgramState *getProgramState() {
    return PState;
  }

  ProcessState *getProcessState() {
    return static_cast<ProcessState*>(getState());
  }

  void setProgramState(ProgramState *PS, int Rank);

  protected:
  void initializeMPICalls();
};

}

#endif // AISLINN_INTERPRETERMPI
