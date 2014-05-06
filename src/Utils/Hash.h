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
/// \brief Hashing of basic types
///
//===----------------------------------------------------------------------===//

#ifndef AISLINN_HASH
#define AISLINN_HASH

#include <mhash.h>

namespace llvm {
  class APInt;
  class GenericValue;
};

namespace aislinn {

inline void hash(MHASH HashThread, size_t S) {
  mhash(HashThread, &S, sizeof(S));
}

void hash(MHASH HashThread, const llvm::APInt &IntVal);
void hash(MHASH HashThread, const llvm::GenericValue &GV);

}

#endif
