//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MPI_PROGRAM_STATE
#define AISLINN_MPI_PROGRAM_STATE

#include <vector>

#include "ProcessState.h"
#include "../Utils/HashDigest.h"

namespace aislinn {

class InterpreterMPI;

class ProgramState
{

  std::vector<ProcessState*> Processes;
  std::deque<Ref<Message> > Messages;

  public:
  explicit ProgramState(int ProcessCount);
  ProgramState(const ProgramState *PState);
  ~ProgramState();

  void callFunctionOnAllProcesses(
      InterpreterMPI *Intr,
      llvm::Function *Fn,
      const std::vector<llvm::GenericValue> &Args);

  void callFunctionAsMainOnAllProcesses(
      InterpreterMPI *Intr,
      llvm::Function *Fn,
      const std::vector<std::string> &Args);

  void runInitialization(InterpreterMPI *Intr);

  int getRank(ExecutionState *State) {
    for (size_t i = 0; i < Processes.size(); ++i) {
      if (State == Processes[i]) {
        return i;
      }
    }
    llvm_unreachable("ExecutionState not found in ProgramState");
  }

  int getSize() const {
    return Processes.size();
  }

  ProcessState *getProcessState(int Rank) {
    assert(Rank >= 0 && Rank < Processes.size());
    return Processes[Rank];
  }

  const ProcessState *getProcessState(int Rank) const {
    assert(Rank >= 0 && Rank < Processes.size());
    return Processes[Rank];
  }

  std::deque<Ref<Message> > & getMessages() {
    return Messages;
  }

  void removeMessage(const Message *M) {
    Messages.erase(findMessage(M));
  }

  void addMessage(Message *M) {
    Messages.push_back(M);
  }

  bool checkMessage(const Message *M) const;

  std::deque<Ref<Message> >::const_iterator findMessage(const Message *M) const;
  std::deque<Ref<Message> >::iterator findMessage(const Message *M);

  void collectMessages(int Receiver,
      int Sender,
      int Tag,
      std::vector<Message*> &Out);

  int messageIndex(const Message *M);

  bool isTerminated() const;
  HashDigest computeHash();
  void dump();
};

}

#endif
