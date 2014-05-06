//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MPI_PROCESS_STATE
#define AISLINN_MPI_PROCESS_STATE

#include "../Interpreter/ExecutionState.h"
#include "Request.h"
#include "../Utils/Ref.h"
#include "../Utils/Hash.h"

#include <deque>

namespace aislinn {

enum ProcessStatus {
  PS_WAIT,
  PS_TEST,
  PS_INITED,
  PS_FINISHED
};

class ProgramState;

class ProcessState : public ExecutionState
{

  ProcessStatus Status;
  int ActiveRequest;
  int *FlagPtr;

  std::vector<Ref<Request> > Requests;

  public:
  ProcessState();

  void setWait(int RequestId) {
    setStatus(PS_WAIT, RequestId);
  }

  void setTest(int RequestId, int* FlagPtr) {
    setStatus(PS_TEST, RequestId, FlagPtr);
  }

  void setFinished() {
    Status = PS_FINISHED;
    ActiveRequest = -1;
    FlagPtr = NULL;
  }

  void setStatus(ProcessStatus Status, int RequestId, int *FlagPtr = NULL) {
    assert( \
        RequestId > 0 && \
        RequestId < Requests.size() && \
        Requests[RequestId].get() != NULL);
    this->Status = Status;
    this->ActiveRequest = RequestId;
    this->FlagPtr = FlagPtr;
  }

  ProcessStatus getStatus() const {
    return Status;
  }

  int* getFlagPtr() const {
    return FlagPtr;
  }

  int getActiveRequest() const {
    return ActiveRequest;
  }

  const Request *getRequest(int Id) const {
    assert(Id > 0 && Id < Requests.size());
    return Requests[Id].get();
  }

  void removeRequest(int Id) {
    assert(Id > 0 && Id < Requests.size());
    Requests[Id] = NULL;
  }

  void hash(ProgramState *PState, MHASH HashThread);
  int newRequest(Request *R);
};

}

#endif // AISLINN_MPI_PROCESS_STATE
