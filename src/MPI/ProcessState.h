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
  std::vector<int> ActiveRequests; // Sorted
  int *FlagPtr;

  std::vector<Ref<Request> > Requests;

  public:
  ProcessState();

  void setWait(int RequestId) {
    Status = PS_WAIT;
    ActiveRequests.clear();
    ActiveRequests.push_back(RequestId);
    FlagPtr = NULL;
  }

  /* Requests has to be sorted! */
  void setWait(std::vector<int> Requests) {
    Status = PS_WAIT;
    ActiveRequests = Requests;
    FlagPtr = NULL;
  }

  void setTest(int RequestId, int* FlagPtr) {
    Status = PS_TEST;
    ActiveRequests.clear();
    ActiveRequests.push_back(RequestId);
    this->FlagPtr = FlagPtr;
  }

  void setFinished() {
    Status = PS_FINISHED;
    ActiveRequests.clear();
    FlagPtr = NULL;
  }

  ProcessStatus getStatus() const {
    return Status;
  }

  int* getFlagPtr() const {
    return FlagPtr;
  }

  const std::vector<int> & getActiveRequests() const {
    return ActiveRequests;
  }

  const Request *getRequest(int Id) const {
    assert(Id > 0 && Id < Requests.size());
    return Requests[Id].get();
  }

  int findRequestId(const Request *R) {
    for (size_t i = 0; i < Requests.size(); i++) {
      if (Requests[i].get() == R) {
        return i;
      }
    }
    return -1;
  }

  void setRequest(int Id, Request *R) {
    assert(Id > 0 && Id < Requests.size());
    Requests[Id] = R;
  }

  void removeRequest(int Id) {
    assert(Id > 0 && Id < Requests.size());
    Requests[Id] = NULL;
  }

  void removeRequests(const std::vector<int> &Ids) {
    for (size_t i = 0; i < Ids.size(); i++) {
      removeRequest(Ids[i]);
    }
  }

  void hash(ProgramState *PState, MHASH HashThread);
  int newRequest(Request *R);
};

}

#endif // AISLINN_MPI_PROCESS_STATE
