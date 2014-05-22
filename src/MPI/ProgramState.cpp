//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "ProgramState.h"
#include "InterpreterMPI.h"
#include "../../include/mpi.h"

using namespace aislinn;

ProgramState::ProgramState(int ProcessCount)
    : Processes(ProcessCount)
{
  assert(ProcessCount >= 1);
  for (int i = 0; i < ProcessCount; i++) {
    Processes[i] = new ProcessState();
  }
}

ProgramState::ProgramState(const ProgramState *PState)
  : Messages(PState->Messages)
{
  for (size_t i = 0, s = PState->Processes.size(); i < s; ++i) {
    Processes.push_back(new ProcessState(*PState->getProcessState(i)));
  }
}

ProgramState::~ProgramState()
{
  for (size_t i = 0; i < Processes.size(); i++) {
    delete Processes[i];
  }
}

void ProgramState::runInitialization(InterpreterMPI *Intr)
{
  for (size_t i = 0; i < Processes.size(); i++) {
    Intr->setProgramState(this, Processes[i]);
    Intr->runInitialization();
  }
}

bool ProgramState::isTerminated() const
{
  for (size_t i = 0; i < Processes.size(); i++) {
    if (Processes[i]->getStatus() != PS_FINISHED) {
      return false;
    }
  }
  return true;
}

void ProgramState::callFunctionOnAllProcesses(
      InterpreterMPI *Intr,
      llvm::Function *Fn,
      const std::vector<llvm::GenericValue> &Args)
{
  for (size_t i = 0; i < Processes.size(); i++) {
    Intr->setState(Processes[i]);
    Intr->callFunction(Fn, Args);
    Intr->run();
  }
}

void ProgramState::callFunctionAsMainOnAllProcesses(
      InterpreterMPI *Intr,
      llvm::Function *Fn,
      const std::vector<std::string> &Args)
{
  for (size_t i = 0; i < Processes.size(); i++) {
    Intr->setState(Processes[i]);
    Intr->callFunctionAsMain(Fn, Args);
    Intr->run();
  }
}

void ProgramState::collectMessagesHelper(
    int Receiver,
    const std::vector<const Request*> &RecvRequests,
    const std::vector<bool> &MustMatch,
    int Index,
    std::vector<Message*> &Matched,
    std::vector<std::vector<Message*> >& Out)
{
  if (Index == RecvRequests.size()) {
    Out.push_back(Matched);
    return;
  }

  const Request *R = RecvRequests[Index];

  bool flags[getSize()];
  for (int i = 0; i < getSize(); i++) {
    flags[i] = false;
  }
  bool matched = false;

  for (size_t i = 0; i < Messages.size(); i++) {
    Message *M = Messages[i].get();
    if (M->Receiver == Receiver && \
       (R->Receive.Sender == MPI_ANY_SOURCE || M->Sender == R->Receive.Sender) && \
       (M->Tag == R->Receive.Tag) && \
       !flags[M->Sender]) {
        size_t j;
        for (j = 0; j < Matched.size(); j++) {
          if (Matched[j] == M)
            break;
        }
        if (j != Matched.size()) {
          continue;
        }
        matched = true;
        flags[M->Sender] = true;
        Matched.push_back(M);
        collectMessagesHelper(Receiver, RecvRequests, MustMatch, Index + 1, Matched, Out);
        Matched.pop_back();
    }
  }

  if (!matched) {
    if (MustMatch[Index]) {
      return;
    }
    Matched.push_back(NULL);
    collectMessagesHelper(Receiver, RecvRequests, MustMatch, Index + 1, Matched, Out);
    Matched.pop_back();
  }
}

void ProgramState::dump()
{
  for (int i = 0; i < Processes.size(); i++) {
    llvm::errs() << "Rank=" << i << " Status=" << Processes[i]->getStatus() << "\n";
  }
}

std::deque<Ref<Message> >::const_iterator
ProgramState::findMessage(const Message *M) const
{
  std::deque<Ref<Message> >::const_iterator I, E = Messages.end();
  for (I = Messages.begin(); I != E; ++I) {
    if (I->get() == M) {
      return I;
    }
  }
  llvm_unreachable("Message not found\n");
}

std::deque<Ref<Message> >::iterator
ProgramState::findMessage(const Message *M)
{
  std::deque<Ref<Message> >::iterator I, E = Messages.end();
  for (I = Messages.begin(); I != E; ++I) {
    if (I->get() == M) {
      return I;
    }
  }
  llvm_unreachable("Message not found\n");
}

bool ProgramState::checkMessage(const Message *M) const
{
  std::deque<Ref<Message> >::const_iterator I, E = Messages.end();
  for (I = Messages.begin(); I != E; ++I) {
    if (I->get() == M) {
      return true;
    }
  }
  return false;
}

/* We sort messages and requests because of hashing they are
 * partly order-independant. This should be solved by
 * right data structure! */

static bool message_sort_helper (const Ref<Message> &m1,
                                 const Ref<Message> &m2)
{
  return m1->Receiver < m2->Receiver ||
         (m1->Receiver == m2->Receiver && m2->Sender < m2->Sender);
}

HashDigest ProgramState::computeHash()
{
  MHASH HashThread = mhash_init(MHASH_MD5);
  if (HashThread == MHASH_FAILED) {
    llvm_unreachable("mhash_init failed\n");
  }

  std::stable_sort(Messages.begin(), Messages.end(), message_sort_helper);

  size_t S = Messages.size();
  mhash(HashThread, &S, sizeof(S));

  for (size_t i = 0; i < Messages.size(); i++) {
    Messages[i]->hash(HashThread);
  }

  for (size_t i = 0; i < Processes.size(); i++) {
    Processes[i]->hash(this, HashThread);
  }

  return HashDigest(HashThread);
}

int ProgramState::messageIndex(const Message *M)
{
  int Index = 0;
  for (int i = 0; i < Messages.size(); i++) {
    if (Messages[i].get() == M) {
      return Index;
    }
    if (Messages[i]->Receiver == M->Receiver && \
        Messages[i]->Sender == M->Sender) {
      Index++;
    }
  }
  return -1;
}
