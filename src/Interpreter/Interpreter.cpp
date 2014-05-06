//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file implements the top-level functionality for the LLVM
/// interpreter.
/// This code is just a slightly modified version of LLVM intepreter.
///
//===----------------------------------------------------------------------===//

#include "Interpreter.h"
#include "llvm/CodeGen/IntrinsicLowering.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"

#include <cstring>

using namespace llvm;
using namespace aislinn;

extern "C" void LLVMLinkInInterpreter() { }

//===----------------------------------------------------------------------===//
// Interpreter ctor - Initialize stuff
//
Interpreter::Interpreter(Module *M)
  : ExecutionEngine(M), State(NULL), MainModule(M), TD(M), PauseFlag(false) {

  memset(&ExitValue.Untyped, 0, sizeof(ExitValue.Untyped));
  setDataLayout(&TD);
  // Initialize the "backend"
  initializeExecutionEngine();
  initializeExternalFunctions();

  IL = new IntrinsicLowering(TD);
}

Interpreter::~Interpreter() {
  delete IL;
}

void Interpreter::runAtExitHandlers () {
  while (!AtExitHandlers.empty()) {
    callFunction(AtExitHandlers.back(), std::vector<GenericValue>());
    AtExitHandlers.pop_back();
    run();
  }
}

/// run - Start execution with the specified function and arguments.
///
GenericValue
Interpreter::runFunction(Function *F,
                         const std::vector<GenericValue> &ArgValues) {
  assert (F && "Function *F was null at entry to run()");

  // Try extra hard not to pass extra args to a function that isn't
  // expecting them.  C programmers frequently bend the rules and
  // declare main() with fewer parameters than it actually gets
  // passed, and the interpreter barfs if you pass a function more
  // parameters than it is declared to take. This does not attempt to
  // take into account gratuitous differences in declared types,
  // though.
  std::vector<GenericValue> ActualArgs;
  const unsigned ArgCount = F->getFunctionType()->getNumParams();
  for (unsigned i = 0; i < ArgCount; ++i)
    ActualArgs.push_back(ArgValues[i]);

  // Set up the function call.
  callFunction(F, ActualArgs);

  // Start executing the function.
  run();

  return ExitValue;
}

void
Interpreter::callFunctionAsMain(
    Function *Fn,
    const std::vector<std::string> &args)
{
  std::vector<llvm::GenericValue> A;
  FunctionType *FTy = Fn->getFunctionType();
  Type* PPInt8Ty = Type::getInt8PtrTy(Fn->getContext())->getPointerTo();
  unsigned NumArgs = Fn->getFunctionType()->getNumParams();
  if (NumArgs == 2) {
    if (FTy->getParamType(1) != PPInt8Ty ||
        !FTy->getParamType(0)->isIntegerTy(32)) {
      report_fatal_error("Invalid type of arguments for main function");
    }

    GenericValue Argc;
    Argc.IntVal = APInt(32, args.size());
    A.push_back(Argc); // Arg #0 = argc.

    size_t S = 0;
    for (size_t i = 0; i < args.size(); i++) {
      S += args[i].size() + 1;
    }

    void **ArgvList = static_cast<void**>
      (State->alloc(sizeof(void*) * (args.size() + 1)));
    char *P = static_cast<char*>(State->alloc(S));

    for (size_t i = 0; i < args.size(); i++) {
      ArgvList[i] = P;
      strcpy(P, args[i].c_str());
      P += args[i].size() + 1;
    }
    ArgvList[args.size()] = NULL;
    A.push_back(PTOGV(ArgvList));
  } else if (NumArgs != 0) {
    report_fatal_error("Invalid number arguments for main function");
  }
  callFunction(Fn, A);
}

void Interpreter::runInitialization() {
  for (Module::const_global_iterator I = MainModule->global_begin(),
       E = MainModule->global_end();
      I != E; ++I) {

     // TODO: Share constant values
    if (!I->isDeclaration()) {
      Type *ElTy = I->getType()->getElementType();
      size_t GVSize = (size_t) TD.getTypeAllocSize(ElTy);
      void *Mem = State->alloc(GVSize);
      State->setGlobalMapping(I, Mem);
    } else {
      // External variable reference. Try to use the dynamic loader to
      // get a pointer to it.
      if (void *SymAddr =
          sys::DynamicLibrary::SearchForAddressOfSymbol(I->getName())) {
      // FIXME: Resolve declaration separately
        State->setGlobalMapping(I, SymAddr);
      } else {
        report_fatal_error("Could not resolve external global address: "
            +I->getName());
      }
    }
  }

  // Now that all of the globals are set up in memory, loop through them all
  // and initialize their contents.
  for (Module::const_global_iterator I = MainModule->global_begin(),
       E = MainModule->global_end();
      I != E; ++I) {
    if (!I->isDeclaration()) {
      void *Addr = State->getPointerToGlobalValue(I);
      InitializeMemory(I->getInitializer(), Addr);
    }
  }
}
