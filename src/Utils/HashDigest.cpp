//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief The class representings hash digest
///
//===----------------------------------------------------------------------===//

#include "HashDigest.h"

using namespace aislinn;

static const char HexChars[16] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

void HashDigest::toString(char *Out) const
{
  size_t Size = size();
  if (Ptr == NULL) {
    strncpy(Out, "<NULL>", Size);
  }
  for (size_t i = 0; i < Size; i++) {
    char Byte = ((char*) Ptr)[i];
    Out[i*2] = HexChars[(Byte & 0xF0) >> 4];
    Out[i*2+1] = HexChars[Byte & 0x0F];
  }
  Out[Size*2] = 0;
}
