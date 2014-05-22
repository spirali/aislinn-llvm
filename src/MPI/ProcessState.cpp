//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "ProcessState.h"
#include <algorithm>

using namespace aislinn;

ProcessState::ProcessState()
  : ExecutionState()
{
  Status = PS_INITED;
  FlagPtr = NULL;
}

void ProcessState::hash(ProgramState *PState, MHASH HashThread)
{
  ExecutionState::hash(HashThread);
  mhash(HashThread, &Status, sizeof(Status));
  for (size_t i = 0; i < ActiveRequests.size(); i++) {
    mhash(HashThread, &ActiveRequests[i], sizeof(int));
  }
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
  int S = Requests.size();
  if (S == 0 || Requests[S - 1].get() != NULL) {
    Requests.push_back(R);
    return S;
  }

  S-=2;
  while(S >= 0 && Requests[S].get() == NULL) {
    S--;
  }
  S++;
  Requests[S] = R;
  return S;
}

/*std::vector<int> ProcessState::getPreRequests(const std::vector<int>& RequestsIds)
{
  std::vector<int> Out;
  int MaxRecvId = -1;
  for (size_t j = 0; j < RequestsIds.size(); j++) {
    int ID = RequestsIds[j];
    Requests *R = Requests[ID];
    if (R->Type == REQUEST_RECV) {
      MaxRecvId = ID;
    }
  }

  if (MaxRecvId == -1) {
    return Out;
  }


  for (int i = 0; i < MaxRecvId; i++) {
    if (
  }


  // It is assumed that RequestsIds is sorted
  if (RequestsIds.size() == 0) {
    return;
  }
  int MaxId = RequestsIds[RequestsIds.size() - 1];

  for (size_t j = 0; j < RequestsIds.size(); j++) {
    int ID = RequestsIds[j];
    for (size_t i = 0; i < ID; i++) {
      if (RequestsIds.find(i) != RequestsIds.end()) {
        continue;
      }
      if (R->dependsOn(Requests[i])) {
        if (Out.find(i) != Out.end()) {
          continue;
        }
        Out.push_back(Requests[i]);
      }
    }
  }
  return Out;
}*/
