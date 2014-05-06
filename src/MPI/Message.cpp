//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "Message.h"

using namespace aislinn;

Message::Message(const void *Data, size_t Size) {
  this->Size = Size;
  this->Data = operator new(Size);
  memcpy(this->Data, Data, Size);
}

Message::~Message() {
  operator delete(Data);
}

void Message::hash(MHASH HashThread) const
{
  mhash(HashThread, &Receiver, sizeof(Receiver));
  mhash(HashThread, &Sender, sizeof(Sender));
  mhash(HashThread, &Tag, sizeof(Tag));
  mhash(HashThread, &Size, sizeof(Size));
  mhash(HashThread, Data, Size);
}
