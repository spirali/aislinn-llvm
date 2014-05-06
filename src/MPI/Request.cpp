//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "Request.h"
#include "ProgramState.h"

using namespace aislinn;

void Request::hash(ProgramState *PState, MHASH HashThread)
{
  mhash(HashThread, &Type, sizeof(Type));
  switch (Type) {
    case REQUEST_RECV:
      mhash(HashThread, &Receive.Sender, sizeof(Receive.Sender));
      mhash(HashThread, &Receive.Tag, sizeof(Receive.Tag));
      mhash(HashThread, &Receive.Data, sizeof(Receive.Data));
      mhash(HashThread, &Receive.Size, sizeof(Receive.Size));
      return;
    case REQUEST_SSEND: {
      int Index = PState->messageIndex(Msg.get());
      mhash(HashThread, &Index, sizeof(Index));
      return;
    }
    default:
        llvm_unreachable("Not implemented request\n");
  }
}

void Request::dump()
{
  if (Type == REQUEST_RECV) {
    llvm::errs() << "Receive\n";
  } else {
    llvm::errs() << "Send\n";
  }
}
