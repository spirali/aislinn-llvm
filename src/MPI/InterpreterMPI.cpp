//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "InterpreterMPI.h"
#include "ProgramState.h"
#include "../MManagment/MemoryManager.h"
#include "../Utils/Debug.h"

using llvm::GenericValue;
using llvm::FunctionType;
using llvm::APInt;
using llvm::errs;
using namespace aislinn;

typedef GenericValue (*ExFunc)(FunctionType *,
                               const std::vector<GenericValue> &);
extern std::map<std::string, ExFunc> FuncNames;

InterpreterMPI::InterpreterMPI(llvm::Module *M) : Interpreter(M)
{
  initializeMPICalls();
}

extern Interpreter *TheInterpreter;
static inline InterpreterMPI* getInterpreter() {
  return static_cast<InterpreterMPI*>(TheInterpreter);
}

#define MAKE_INT(x) APInt(sizeof(int) * 8, x, true);

static GenericValue lle_X_MPI_Comm_rank(
    FunctionType *FT,
    const std::vector<GenericValue> &Args)
{
  assert(Args.size() == 2);
  ProgramState *PState = getInterpreter()->getProgramState();
  ExecutionState *State = getInterpreter()->getState();

  int *Rank = static_cast<int*>(Args[1].PointerVal);
  *Rank = PState->getRank(State);

  GenericValue GV;
  GV.IntVal = MAKE_INT(0);
  return GV;
}

static GenericValue lle_X_MPI_Isend(FunctionType *FT,
                               const std::vector<GenericValue> &Args) {
  assert(Args.size() == 7);

  void *Data = static_cast<void*>(Args[0].PointerVal);
  int Count = static_cast<int>(Args[1].IntVal.getZExtValue());
  size_t Size = Count * sizeof(int);
  int Target = static_cast<int>(Args[3].IntVal.getZExtValue());
  int Tag = static_cast<int>(Args[4].IntVal.getZExtValue());
  int *RequestIdPtr = static_cast<int*>(Args[6].PointerVal);

  ProgramState *PState = getInterpreter()->getProgramState();
  ProcessState *State = getInterpreter()->getProcessState();

  Message *M = new Message(Data, Size);
  M->Receiver = Target;
  M->Sender = PState->getRank(State);
  M->Tag = Tag;
  PState->addMessage(M);

  Request *R = new Request;
  R->Type = REQUEST_SSEND;
  R->Msg = Ref<Message>(M);
  int Id = State->newRequest(R);
  *RequestIdPtr = Id;
  //memcpy(RequestPtr, &R, sizeof(void*));

  /*if (State2->Status != PS_RECEIVE_PAUSE || State2->OtherProcess != State) {
    State->Status = PS_SEND_PAUSE;
    State->OtherProcess = State2;
    State->Data = Data;
    getInterpreter()->pause();
    return GenericValue();
  }

  State2->Status = PS_RECEIVE_READY;
  State2->OtherProcess = NULL;
  State2->Data = Data;*/

  //GV.IntVal = MAKE_INT(PState->getRank(State));
  //
  //getInterpreter()->pause();
  GenericValue GV;
  GV.IntVal = MAKE_INT(0);
  return GV;
}

static GenericValue lle_X_MPI_Irecv(FunctionType *FT,
                               const std::vector<GenericValue> &Args) {
  //ProgramState *PState = getInterpreter()->getProgramState();

  assert(Args.size() == 7);

  void *Data = static_cast<void*>(Args[0].PointerVal);
  int Count = static_cast<int>(Args[1].IntVal.getZExtValue());
  size_t Size = Count * sizeof(int);
  int Sender = static_cast<int>(Args[3].IntVal.getZExtValue());
  int Tag = static_cast<int>(Args[4].IntVal.getZExtValue());
  int *RequestIdPtr = static_cast<int*>(Args[6].PointerVal);

  ProcessState *State = getInterpreter()->getProcessState();
  ProgramState *PState = getInterpreter()->getProgramState();
  Request *R = new Request;
  R->Type = REQUEST_RECV;
  R->Receive.Sender = Sender;
  R->Receive.Tag = Tag;
  R->Receive.Data = Data;
  R->Receive.Size = Size;

  //(*RequestPtr) = R;
  int Id = State->newRequest(R);
  *RequestIdPtr = Id;

  /*int From = (int) Args[0].IntVal.getZExtValue();
  ProcessState *State2 = PState->getProcessState(From);

  GenericValue GV;
  if (State2->Status != PS_SEND_PAUSE || State2->OtherProcess != State) {
    State->Status = PS_RECEIVE_PAUSE;
    State->OtherProcess = State2;
    getInterpreter()->pause();
    return GV;
  }

  State2->Status = PS_SEND_READY;
  State2->OtherProcess = NULL;
  GV.IntVal = MAKE_INT(State2->Data);
  return GV;*/
  GenericValue GV;
  GV.IntVal = MAKE_INT(0);
  return GV;
}

static GenericValue lle_X_MPI_Wait(FunctionType *FT,
                               const std::vector<GenericValue> &Args) {
  //ProgramState *PState = getInterpreter()->getProgramState();

  assert(Args.size() == 2);

  /*void *Data = static_cast<void*>(Args[0].PointerVal);
  int Count = static_cast<int>(Args[1].IntVal.getZExtValue());
  size_t Size = Count * sizeof(int);
  int Sender = static_cast<int>(Args[3].IntVal.getZExtValue());
  int Tag = static_cast<int>(Args[4].IntVal.getZExtValue());*/
  int *RequestIdPtr = static_cast<int*>(Args[0].PointerVal);

  /*RecvRequest *R = new RecvRequest(Sender, Tag, Data, Size);
  *RequestPtr = R;*/
  ProcessState *State = getInterpreter()->getProcessState();
  State->setWait(*RequestIdPtr);
  getInterpreter()->pause();

  GenericValue GV;
  GV.IntVal = MAKE_INT(0);
  return GV;
}

static GenericValue lle_X_MPI_Test(FunctionType *FT,
                               const std::vector<GenericValue> &Args) {
  assert(Args.size() == 3);
  int *RequestIdPtr = static_cast<int*>(Args[0].PointerVal);
  int *FlagPtr = static_cast<int*>(Args[1].PointerVal);
  *FlagPtr = 0; // Initialize to 0, so if the message is not ready we can directly continue
  ProcessState *State = getInterpreter()->getProcessState();
  State->setTest(*RequestIdPtr, FlagPtr);
  getInterpreter()->pause();

  GenericValue GV;
  GV.IntVal = MAKE_INT(0);
  return GV;
}



void InterpreterMPI::initializeMPICalls()
{
  FuncNames["lle_X_MPI_Comm_rank"] = lle_X_MPI_Comm_rank;
  FuncNames["lle_X_MPI_Isend"]     = lle_X_MPI_Isend;
  FuncNames["lle_X_MPI_Irecv"]     = lle_X_MPI_Irecv;
  FuncNames["lle_X_MPI_Wait"]      = lle_X_MPI_Wait;
  FuncNames["lle_X_MPI_Test"]      = lle_X_MPI_Test;
}

void InterpreterMPI::setProgramState(ProgramState *PS, int Rank)
{
  ProcessState *S = PS->getProcessState(Rank);
  IFVERBOSE(1) {
    errs() << "Set program state=" << PS << " process state=" << S << " rank=" << Rank << "\n";
  }
  PState = PS;
  setState(S);
}
