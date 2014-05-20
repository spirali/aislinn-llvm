//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_EXECUTION_STATE_H
#define AISLINN_EXECUTION_STATE_H

#include "../MManagment/MemoryObjects.h"
#include "llvm/IR/Function.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/CallSite.h"

#include <map>

namespace llvm {
class Module;
}

namespace aislinn {

class ExecutionState;

class ExecutionContext {
  public:
  llvm::Function             *CurFunction;// The currently executing function
  llvm::BasicBlock           *CurBB;      // The currently executing BB
  llvm::BasicBlock::iterator  CurInst;    // The next instruction to execute
  std::map<llvm::Value *, llvm::GenericValue> Values; // LLVM values used in this invocation
  std::vector<llvm::GenericValue>  VarArgs; // Values passed through an ellipsis
  llvm::CallSite             Caller;     // Holds the call that called subframes.
                                   // NULL if main func or debugger invoked fn
  std::vector<void*> Allocas;    // Track memory allocated by alloca

  void free(ExecutionState *State);
};

class ExecutionState {
  std::vector<ExecutionContext> ECStack;
  MemoryObjects MObjects;

  std::map<llvm::GlobalValue *, void *> GlobalMapping;

  public:
  ExecutionState();

  std::vector<ExecutionContext>& getECStack() {
    return ECStack;
  }

  void makeActive() {
    MObjects.makeActive();
  }

  void setGlobalMapping(const llvm::GlobalValue *GV, void *Pointer);
  void* getPointerToGlobalValue(const llvm::GlobalValue *GV);

  void *alloc(size_t Size) {
    return MObjects.alloc(Size);
  }

  void free(const void *Address) {
    // TMP HACK: Remove const_cast
    MObjects.free(const_cast<void*>(Address));
  }

  void hash(MHASH HashThread);

  void dump() {
    MObjects.dump();
  }

  protected:

  void* allocMemoryForGV(const llvm::GlobalVariable *GV);
};

}

#endif // AISLINN_EXECUTION_STATE_H
