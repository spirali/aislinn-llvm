//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_REQUEST
#define AISLINN_REQUEST

#include "Message.h"
#include "llvm/Support/raw_ostream.h"

namespace aislinn {

enum RequestType {
  REQUEST_SEND,
  REQUEST_SSEND,
  REQUEST_RECV,
  REQUEST_RECV_COMPLETED,
};

class ProgramState;
class StateSpace;

struct Request : public RefCountedObj {
  RequestType Type;
  Ref<Message> Msg;
  union {
    struct {
      int Sender;
      int Tag;
      void *Data;
      size_t Size;
    } Receive;
  };

  bool dependsOn(Request *R);
  void dump();
  void hash(ProgramState *PState, MHASH HashThread);
};

}

#endif
