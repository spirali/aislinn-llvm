//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MPI_MESSAGE
#define AISLINN_MPI_MESSAGE

#include "../Utils/Hash.h"
#include "../Utils/Ref.h"

namespace aislinn {

struct Message : public RefCountedObj {

  Message(const void *Data, size_t Size);
  ~Message();
  int Receiver;
  int Sender;
  int Tag;
  void *Data;
  size_t Size;

  void hash(MHASH HashThread) const;

};

}

#endif // AISLINN_MPI_MESSAGE
