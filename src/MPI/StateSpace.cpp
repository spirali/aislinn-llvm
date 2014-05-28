//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "StateSpace.h"
#include "ProgramState.h"
#include "InterpreterMPI.h"
#include "Node.h"
#include "../Utils/XML.h"
#include "../Utils/Debug.h"
#include <stdio.h>
#include <sstream>

using namespace aislinn;

ErrorTy::ErrorTy(Node *ErrorNode) : ErrorNode(ErrorNode)
{

}

ErrorTy::~ErrorTy()
{

}

void ErrorTy::writeReport(StateSpace &SSpace, XML &Report)
{
    Report.child("error");
    writeReportBody(SSpace, Report);
    Report.back(); // error
}

class DeadlockError : public ErrorTy
{
  public:
    DeadlockError(Node *ErrorNode): ErrorTy(ErrorNode) {}

    void writeReportBody(StateSpace &SSpace, XML &Report) {
      Report.set("type", "deadlock");
      Report.simpleChild("description", "Deadlock");
      SSpace.writePathToNode(ErrorNode, Report);
    }
};

class NonzeroExitCodeError : public ErrorTy
{
  int Rank;
  int ExitCode;

  public:
    NonzeroExitCodeError(Node *ErrorNode, int Rank, int ExitCode) :
      ErrorTy(ErrorNode), Rank(Rank), ExitCode(ExitCode) {}

    void writeReportBody(StateSpace &SSpace, XML &Report) {
      Report.set("type", "exitcode");
      Report.set("rank", Rank);
      Report.set("exitcode", ExitCode);
      std::stringstream Description;
      Description << "Rank " << Rank << " exited with code " << ExitCode;
      Report.simpleChild("description", Description.str());
      SSpace.writePathToNode(ErrorNode, Report);
    }
};

StateSpace::~StateSpace()
{
  std::map<HashDigest, Node*>::iterator I, E;
  for (I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    delete I->second;
  }

  for (size_t i = 0; i < Errors.size(); i++) {
    delete Errors[i];
  }
}

StateSpace::StateSpace(
    InterpreterMPI *Intr,
    ProgramState *PState,
    const std::vector<std::string> &Args)
  : Intr(Intr)
{
  llvm::Function *MainFunction = Intr->getMainModule()->getFunction("main");
  if (!MainFunction) {
    llvm::errs() << "function 'main' not found in module.\n";
    exit(1);
  }

  InitialNode = new Node();

  for (int i = 0; i < PState->getSize(); i++) {
    Intr->setProgramState(PState, i);
    Intr->callFunctionAsMain(MainFunction, Args);
    Intr->run();
    afterRunAction(NULL);
  }

  Node *N = getNode(PState);
  InitialNode->addArc(Arc(N, PState->getActions()));
  PState->clearActions();
}

void StateSpace::build()
{
  while(!WorkingQueue.empty()) {
    std::pair<Node *, ProgramState*> V = WorkingQueue.front();
    WorkingQueue.pop_front();
    computeChilds(V.first, V.second);
  }
}

Node* StateSpace::getNode(ProgramState *PState)
{
  HashDigest Hash = PState->computeHash();
  std::map<HashDigest, Node*>::iterator I = Nodes.find(Hash);

  if (I == Nodes.end()) {
    // TODO: Share hash in Node with Hash as key in map
    Node *N = new Node(Hash);
    Nodes[Hash] = N;
    WorkingQueue.push_back(std::pair<Node*, ProgramState*>(N, PState));
    return N;
  }

  Node *N = I->second;
  return N;
}

static void writeNode(FILE *f, Node *N, char *HashString)
{
  N->getHash().toString(HashString);
  HashString[6] = 0;
  fprintf(f, "S%p [label=\"%s\"]\n", N, HashString);
  size_t S = N->getArcs().size();
  for (size_t j = 0; j < S; ++j) {
    const Arc &A = N->getArcs()[j];
    int V = -1;
    if (A.Actions.size()) {
      V = A.Actions[0].getRank();
    }
    fprintf(f, "S%p -> S%p [label=\"%i/%lu\"]\n",
        N, A.TargetNode, V, A.Actions.size());
  }

}

void StateSpace::writeDotFile(const std::string &Filename) const
{
  IFVERBOSE(2) {
    llvm::errs() << "Writing .dot file\n";
  }

  FILE *f = fopen(Filename.c_str(), "w");
  fprintf(f, "digraph X {\n");
  std::map<HashDigest, Node*>::const_iterator I, E;
  char *HashString = ALLOCA_STRING_FOR_HASH;
  writeNode(f, InitialNode, HashString); // Initial node has not hash
  for (I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    Node *N = I->second;
    writeNode(f, N, HashString);
  }
  fprintf(f, "}\n");
  fclose(f);
}

bool StateSpace::checkFastRunWait(
  Node *RootNode,
  ProgramState *PState,
  int Rank)
{
  ProcessState *State = PState->getProcessState(Rank);
  const std::vector<int> &RequestIds = State->getActiveRequests();

  for (size_t i = 0; i < RequestIds.size(); i++) {
    const Request *R = State->getRequest(RequestIds[i]);
    switch (R->Type) {
      case REQUEST_SSEND:
        if (PState->checkMessage(R->Msg.get())) {
          return false; // Message was no received yet
        }
        break;
      case REQUEST_RECV_COMPLETED:
        break;
      default:
        return false;
    }
  }

  IFVERBOSE(2) {
    llvm::errs() << "Fast run on rank " << Rank << " Wait / SSEND\n";
  }

  Intr->setProgramState(PState, Rank);
  State->removeRequests(RequestIds);
  llvm::GenericValue GV;
  GV.IntVal = llvm::APInt(sizeof(int) * 8, 0);
  resumeRun(RootNode, GV);

  Node *N = getNode(PState);
  RootNode->addArc(Arc(N, PState->getActions()));
  PState->clearActions();
  return true;
}

void StateSpace::forkWaitOrTest(
    Node *RootNode,
    ProgramState *PState,
    int Rank,
    ProcessStatus Status)
{
  if (Status == PS_TEST) {
    IFVERBOSE(2) {
      llvm::errs() << "Fork on rank " << Rank << " PS_TEST / flag = 0\n";
    }
    ProgramState *NewState = new ProgramState(PState);
    Intr->setProgramState(NewState, Rank);

    llvm::GenericValue GV;
    GV.IntVal = llvm::APInt(sizeof(int) * 8, 0);
    resumeRun(RootNode, GV);
    Node *N = getNode(NewState);

    RootNode->addArc(Arc(N, PState->getActions()));
    PState->clearActions();
  }

  ProcessState *State = PState->getProcessState(Rank);
  const std::vector<int> &RequestIds = State->getActiveRequests();

  std::vector<std::vector <Message*> > MessageMatches;

  int MaxRecvId = -1;

  for (size_t i = 0; i < RequestIds.size(); i++) {
    const Request *R = State->getRequest(RequestIds[i]);
    switch (R->Type) {
      case REQUEST_SSEND:
        if (PState->checkMessage(R->Msg.get())) {
          return; // Message was no received yet
        }
        break;
      case REQUEST_RECV_COMPLETED:
        break;
      case REQUEST_RECV:
        MaxRecvId = RequestIds[i];
        break;
      default:
        llvm_unreachable("Unknown request type in checkForWaitTest");
    }
  }

  std::vector<const Request* > ReceiveRequests;
  std::vector<bool> MustMatch;
  if (MaxRecvId >= 0) {
    for (int i = 0; i <= MaxRecvId; i++) {
      const Request *R = State->getRequest(i);
      if (R == NULL) {
	continue;
      }
      if (R->Type == REQUEST_RECV) {
        ReceiveRequests.push_back(R);
        MustMatch.push_back(std::find(RequestIds.begin(),
                                          RequestIds.end(),
                                          i) != RequestIds.end());
      }
    }
    PState->collectMessages(Rank, ReceiveRequests, MustMatch, MessageMatches);
  } else {
    MessageMatches.push_back(std::vector<Message*>());
  }

  for (int i = 0; i < MessageMatches.size(); i++) {
    std::vector<Message*> &MList = MessageMatches[i];
    IFVERBOSE(2) {
      llvm::errs() << "Fork on rank " << Rank << " PS_TEST|PS_WAIT / RECV\n";
    }

    ProgramState *NewState = new ProgramState(PState);
    Intr->setProgramState(NewState, Rank);

    if (State->getFlagPtr()) {
      *(State->getFlagPtr()) = 1;
    }

    ProcessState *S = NewState->getProcessState(Rank);

    for (size_t j = 0; j < MList.size(); j++) {
      Message* M = MList[j];
      if (M != NULL) {
        const Request *R = ReceiveRequests[j];
        NewState->removeMessage(M);
        memcpy(R->Receive.Data, M->Data, M->Size);
        if (!MustMatch[j]) {
          Request *NewR = new Request(*R);
          NewR->Type = REQUEST_RECV_COMPLETED;
          int Id = S->findRequestId(R);
          assert(Id != -1);
          // We do not wait for this request, so it were not removed by removeRequest
          S->setRequest(Id, NewR);
        }
      }
    }

    S->removeRequests(RequestIds);

    llvm::GenericValue GV;
    GV.IntVal = llvm::APInt(sizeof(int) * 8, 0);
    resumeRun(RootNode, GV);

    Node *N = getNode(NewState);
    RootNode->addArc(Arc(N, PState->getActions()));
    PState->clearActions();
  }
}

void StateSpace::computeChilds(Node *RootNode, ProgramState *PState)
{
  IFVERBOSE(2) {
    llvm::errs() << "Processing node " << this << "\n";
    IFVERBOSE(3) {
      PState->dump();
    }
  }
  std::vector<Message*> Messages;

  llvm::GenericValue GV;
  GV.IntVal = llvm::APInt(sizeof(int) * 8, 0);

  for (int i = 0; i < PState->getSize(); i++) {
    ProcessState *State = PState->getProcessState(i);
    if (State->getStatus() == PS_WAIT) {
      if (checkFastRunWait(RootNode, PState, i)) {
        return;
      }
    }
  }

  for (int i = 0; i < PState->getSize(); i++) {
    ProcessState *State = PState->getProcessState(i);
    ProcessStatus Status = State->getStatus();
    if (Status == PS_WAIT || Status == PS_TEST) {
      forkWaitOrTest(RootNode, PState, i, Status);
    }
  }

  if (RootNode->getArcs().size() == 0 && !PState->isTerminated()) {
    addError(new DeadlockError(RootNode));
  }

  delete PState;
}

void StateSpace::writeReport(XML &Report)
{
  Report.child("statistics");
  Report.simpleChild("nodes", Nodes.size());
  Report.back(); // statistics

  Report.child("analyses");
  Report.child("analysis");
  Report.set("name", "Deadlock");
  Report.back();
  Report.back();

  for (size_t i = 0; i < Errors.size(); i++) {
    Errors[i]->writeReport(*this, Report);
  }
}

void StateSpace::addError(ErrorTy *Error)
{
  Errors.push_back(Error);
}

// This should be called everytime we return from Interpreter
void StateSpace::afterRunAction(Node *N)
{
  if (!Intr->isPaused()) {
    ProcessState *State = Intr->getProcessState();
    State->setFinished();
    int ExitCode = Intr->getExitValue().IntVal.getZExtValue();
    if (ExitCode != 0) {
      ProgramState *PState = Intr->getProgramState();
      addError(new NonzeroExitCodeError(N, PState->getRank(State), ExitCode));
    }
  }
}

void StateSpace::writePathToNode(Node *N, XML &Report)
{
  Report.child("path");
  std::vector<const Arc*> Path;
  for(;;) {
    Node *P = N->getPrevNode();
    if (P == NULL) {
      break;
    }
    const std::vector<Arc> &Arcs = P->getArcs();
    size_t i;
    for (i = 0; i < Arcs.size(); i++) {
      if (Arcs[i].TargetNode == N) {
        break;
      }
    }
    assert(i != Arcs.size());
    Path.push_back(&Arcs[i]);
    N = P;
  };

  std::vector<const Arc*>::reverse_iterator I;
  for (I = Path.rbegin(); I != Path.rend(); ++I) {
    const Arc* A = *I;
    for (size_t i = 0; i < A->Actions.size(); i++) {
      A->Actions[i].write(Report);
    }
  }
  Report.back();
}

