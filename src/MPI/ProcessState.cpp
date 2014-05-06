//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "ProcessState.h"

using namespace aislinn;

ProcessState::ProcessState()
  : ExecutionState()
{
  Status = PS_INITED;
  ActiveRequest = -1;
  FlagPtr = NULL;
}

void ProcessState::hash(ProgramState *PState, MHASH HashThread)
{
  ExecutionState::hash(HashThread);
  mhash(HashThread, &Status, sizeof(Status));
  mhash(HashThread, &ActiveRequest, sizeof(ActiveRequest));
  mhash(HashThread, &FlagPtr, sizeof(FlagPtr));
 for (size_t i = 0; i < Requests.size(); i++) {
    Request *R = Requests[i].get();
    if (R != NULL) {
      mhash(HashThread, &i, sizeof(i));
      R->hash(PState, HashThread);
    }
  }
}

int ProcessState::newRequest(Request *R)
{
  size_t i;
  for (i = 0; i < Requests.size(); i++) {
    if (Requests[i].get() == NULL) {
      //R->Id = i;
      Requests[i] = R;
      return i;
    }
  }
  Requests.push_back(R);
  return i;
}
