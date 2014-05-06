//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "ExecutionState.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/GlobalVariable.h"

using namespace aislinn;

void ExecutionContext::free(ExecutionState *State)
{
  for (std::vector<void*>::iterator I = Allocas.begin(), E = Allocas.end();
       I != E;
       ++I) {
    State->free(*I);
  }
}

ExecutionState::ExecutionState()
{
  // TODO: Delete Stack
}

void ExecutionState::setGlobalMapping(const llvm::GlobalValue *GV, void *Pointer)
{
  GlobalMapping[const_cast<llvm::GlobalValue*>(GV)] = Pointer;
}

void* ExecutionState::getPointerToGlobalValue(const llvm::GlobalValue *GV)
{
  std::map<llvm::GlobalValue*, void*>::iterator I =
    GlobalMapping.find(const_cast<llvm::GlobalValue*>(GV));
  return I != GlobalMapping.end() ? I->second : NULL;
}

void ExecutionState::hash(MHASH HashThread)
{
  MObjects.hash(HashThread);

  for (size_t i = 0; i < ECStack.size(); i++) {
    const ExecutionContext &E = ECStack[i];
    mhash(HashThread, &E.CurFunction, sizeof(void*));
    mhash(HashThread, &E.CurBB, sizeof(void*));
    mhash(HashThread, &E.CurInst, sizeof(void*));
    mhash(HashThread, &E.CurFunction, sizeof(void*));

    std::map<llvm::Value *, llvm::GenericValue>::const_iterator I;
    for (I = E.Values.begin(); I != E.Values.end(); I++) {
      mhash(HashThread, &I->first, sizeof(void*));
      ::hash(HashThread, I->second);
    }

    for (size_t i = 0; i < E.VarArgs.size(); i++) {
      ::hash(HashThread, E.VarArgs[i]);
    }

    for (size_t i = 0; i < E.Allocas.size(); i++) {
      mhash(HashThread, &E.Allocas[i], sizeof(void*));
    }
    // TODO: Hash CurInst && Caller (?)
  }
}
