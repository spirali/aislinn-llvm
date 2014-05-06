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
/// \brief This header file defines the interpreter structure.
///  This code is just a slightly modified version of LLVM intepreter.
///
//===----------------------------------------------------------------------===//
//
#ifndef AISLINN_INTERPRETER_H
#define AISLINN_INTERPRETER_H

#include "ExecutionState.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/InstVisitor.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

class IntrinsicLowering;
struct FunctionInfo;
template<typename T> class generic_gep_type_iterator;
class ConstantExpr;

}

namespace aislinn {

typedef llvm::generic_gep_type_iterator<llvm::User::const_op_iterator> \
        gep_type_iterator;
typedef std::vector<llvm::GenericValue> ValuePlaneTy;

// Interpreter - This class represents the entirety of the interpreter.
//
class Interpreter :
        public llvm::ExecutionEngine,
        public llvm::InstVisitor<Interpreter> {
  ExecutionState *State;
  llvm::Module *MainModule;
  llvm::GenericValue ExitValue;          // The return value of the called function
  llvm::DataLayout TD;
  llvm::IntrinsicLowering *IL;

  // AtExitHandlers - List of functions to call when the program exits,
  // registered with the atexit() library function.
  std::vector<llvm::Function*> AtExitHandlers;

  bool PauseFlag;

public:
  explicit Interpreter(llvm::Module *M);
  ~Interpreter();

  // Aislinn specific functions START ---------------------------------
  //

  llvm::Module* getMainModule() {
    return MainModule;
  }

  std::vector<ExecutionContext>& getECStack() {
    return State->getECStack();
  }

  bool isPaused() const {
    return PauseFlag;
  }

  void pause() {
    PauseFlag = true;
  }

  void setState(ExecutionState *State) {
    if (this->State != State) {
      this->State = State;
      State->makeActive();
    }
  }

  ExecutionState *getState() const {
    return State;
  }

  const llvm::GenericValue& getExitValue() const {
    return ExitValue;
  }

  void resumeRun(const llvm::GenericValue &Value);
  void runInitialization();

  // From ExecutionEngine
  llvm::GenericValue getConstantValue(const llvm::Constant *C);

  // Aislinn specific functions END -----------------------------------

  /// runAtExitHandlers - Run any functions registered by the program's calls to
  /// atexit(3), which we intercept and store in AtExitHandlers.
  ///
  void runAtExitHandlers();

  static void Register() {
    InterpCtor = create;
  }

  /// create - Create an interpreter ExecutionEngine. This can never fail.
  ///
  static ExecutionEngine *create(llvm::Module *M, std::string *ErrorStr = 0);

  /// run - Start execution with the specified function and arguments.
  ///
  llvm::GenericValue runFunction(llvm::Function *F,
                           const std::vector<llvm::GenericValue> &ArgValues);

  void callFunctionAsMain(llvm::Function *Fn, const std::vector<std::string> &args);

  virtual void *getPointerToNamedFunction(const std::string &Name,
                                          bool AbortOnFailure = true) {
    // FIXME: not implemented.
    return 0;
  }

  /// recompileAndRelinkFunction - For the interpreter, functions are always
  /// up-to-date.
  ///
  virtual void *recompileAndRelinkFunction(llvm::Function *F) {
    return getPointerToFunction(F);
  }

  /// freeMachineCodeForFunction - The interpreter does not generate any code.
  ///
  void freeMachineCodeForFunction(llvm::Function *F) { }

  // Methods used to execute code:
  // Place a call on the stack
  void callFunction(llvm::Function *F,
		    const std::vector<llvm::GenericValue> &ArgVals);
  void run();                // Execute instructions until nothing left to do

  // Opcode Implementations
  void visitReturnInst(llvm::ReturnInst &I);
  void visitBranchInst(llvm::BranchInst &I);
  void visitSwitchInst(llvm::SwitchInst &I);
  void visitIndirectBrInst(llvm::IndirectBrInst &I);

  void visitBinaryOperator(llvm::BinaryOperator &I);
  void visitICmpInst(llvm::ICmpInst &I);
  void visitFCmpInst(llvm::FCmpInst &I);
  void visitAllocaInst(llvm::AllocaInst &I);
  void visitLoadInst(llvm::LoadInst &I);
  void visitStoreInst(llvm::StoreInst &I);
  void visitGetElementPtrInst(llvm::GetElementPtrInst &I);
  void visitPHINode(llvm::PHINode &PN) {
    llvm_unreachable("PHI nodes already handled!");
  }
  void visitTruncInst(llvm::TruncInst &I);
  void visitZExtInst(llvm::ZExtInst &I);
  void visitSExtInst(llvm::SExtInst &I);
  void visitFPTruncInst(llvm::FPTruncInst &I);
  void visitFPExtInst(llvm::FPExtInst &I);
  void visitUIToFPInst(llvm::UIToFPInst &I);
  void visitSIToFPInst(llvm::SIToFPInst &I);
  void visitFPToUIInst(llvm::FPToUIInst &I);
  void visitFPToSIInst(llvm::FPToSIInst &I);
  void visitPtrToIntInst(llvm::PtrToIntInst &I);
  void visitIntToPtrInst(llvm::IntToPtrInst &I);
  void visitBitCastInst(llvm::BitCastInst &I);
  void visitSelectInst(llvm::SelectInst &I);


  void visitCallSite(llvm::CallSite CS);
  void visitCallInst(llvm::CallInst &I) {
	  visitCallSite (llvm::CallSite (&I));
  }
  void visitInvokeInst(llvm::InvokeInst &I) {
    visitCallSite (llvm::CallSite (&I));
  }
  void visitUnreachableInst(llvm::UnreachableInst &I);

  void visitShl(llvm::BinaryOperator &I);
  void visitLShr(llvm::BinaryOperator &I);
  void visitAShr(llvm::BinaryOperator &I);

  void visitVAArgInst(llvm::VAArgInst &I);
  void visitExtractElementInst(llvm::ExtractElementInst &I);
  void visitInsertElementInst(llvm::InsertElementInst &I);
  void visitShuffleVectorInst(llvm::ShuffleVectorInst &I);

  void visitExtractValueInst(llvm::ExtractValueInst &I);
  void visitInsertValueInst(llvm::InsertValueInst &I);

  void visitInstruction(llvm::Instruction &I) {
    llvm::errs() << I << "\n";
    llvm_unreachable("Instruction not interpretable yet!");
  }

  llvm::GenericValue callExternalFunction(
                  llvm::Function *F,
                  const std::vector<llvm::GenericValue> &ArgVals);
  void exitCalled(llvm::GenericValue GV);

  void addAtExitHandler(llvm::Function *F) {
    AtExitHandlers.push_back(F);
  }

  llvm::GenericValue *getFirstVarArg () {
    return &(getECStack().back ().VarArgs[0]);
  }

private:  // Helper functions
  llvm::GenericValue executeGEPOperation(llvm::Value *Ptr, gep_type_iterator I,
                                   gep_type_iterator E, ExecutionContext &SF);

  // SwitchToNewBasicBlock - Start execution in a new basic block and run any
  // PHI nodes in the top of the block.  This is used for intraprocedural
  // control flow.
  //
  void SwitchToNewBasicBlock(llvm::BasicBlock *Dest, ExecutionContext &SF);

  void *getPointerToFunction(llvm::Function *F) {
    return (void*)F;
  }

  void *getPointerToBasicBlock(llvm::BasicBlock *BB) {
    return (void*)BB;
  }

  void initializeExecutionEngine() { }
  void initializeExternalFunctions();
  llvm::GenericValue getConstantExprValue(
                  llvm::ConstantExpr *CE, ExecutionContext &SF);
  llvm::GenericValue getOperandValue(llvm::Value *V, ExecutionContext &SF);
  llvm::GenericValue executeTruncInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                ExecutionContext &SF);
  llvm::GenericValue executeSExtInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                               ExecutionContext &SF);
  llvm::GenericValue executeZExtInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                               ExecutionContext &SF);
  llvm::GenericValue executeFPTruncInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                  ExecutionContext &SF);
  llvm::GenericValue executeFPExtInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                ExecutionContext &SF);
  llvm::GenericValue executeFPToUIInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                 ExecutionContext &SF);
  llvm::GenericValue executeFPToSIInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                 ExecutionContext &SF);
  llvm::GenericValue executeUIToFPInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                 ExecutionContext &SF);
  llvm::GenericValue executeSIToFPInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                 ExecutionContext &SF);
  llvm::GenericValue executePtrToIntInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                   ExecutionContext &SF);
  llvm::GenericValue executeIntToPtrInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                   ExecutionContext &SF);
  llvm::GenericValue executeBitCastInst(llvm::Value *SrcVal, llvm::Type *DstTy,
                                  ExecutionContext &SF);
  llvm::GenericValue executeCastOperation(
                  llvm::Instruction::CastOps opcode,
                  llvm::Value *SrcVal,
                  llvm::Type *Ty, ExecutionContext &SF);
  void popStackAndReturnValueToCaller(llvm::Type *RetTy, llvm::GenericValue Result);

};

} // End llvm namespace

#endif
