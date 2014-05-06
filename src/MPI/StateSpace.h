//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_STATESPACE
#define AISLINN_STATESPACE

#include "ProgramState.h"
#include "InterpreterMPI.h"

#include <vector>
#include <deque>
#include <map>


namespace aislinn {

class XML;
class Node;

class ErrorTy {
  Node *ErrorNode;

  public:
    ErrorTy(Node *ErrorNode);
    virtual ~ErrorTy();
    void writeReport(XML &Report);
  protected:
    virtual void writeReportBody(XML &Report) = 0;
};

class StateSpace
{
  std::map<HashDigest, Node*> Nodes;
  std::deque<std::pair<Node*, ProgramState*> > WorkingQueue;
  InterpreterMPI *Intr;
  Node *InitialNode;

  std::vector<ErrorTy*> Errors;

  public:
  StateSpace(InterpreterMPI *Intr,
             ProgramState *PState,
             const std::vector<std::string> &Args);
  ~StateSpace();

  InterpreterMPI *getInterpreter() {
    return Intr;
  }

  void build();
  Node* getNode(ProgramState *PState);


  void writeDotFile(const std::string &Filename) const;
  void writeReport(XML &Report);
  void computeChilds(Node *RootNode, ProgramState *PState);
  void addError(ErrorTy *Error);

  protected:
    void resumeRun(Node *N, const llvm::GenericValue &value) {
      Intr->resumeRun(value);
      afterRunAction(N);
    }

    void afterRunAction(Node *N);

};

}

#endif // AISLINN_STATESPACE
