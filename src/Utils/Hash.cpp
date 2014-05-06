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

#include "Hash.h"
#include "llvm/ExecutionEngine/GenericValue.h"

void aislinn::hash(MHASH HashThread, const llvm::APInt &IntVal)
{
  uint64_t Value = IntVal.getZExtValue();
  mhash(HashThread, &Value, sizeof(Value));
}

void aislinn::hash(MHASH HashThread, const llvm::GenericValue &GV)
{
  mhash(HashThread, &GV.Untyped, sizeof(GV.Untyped));
  hash(HashThread, GV.IntVal);
  if (GV.AggregateVal.size() > 0) {
    hash(HashThread, GV.AggregateVal.size());
    for (size_t i = 0; i < GV.AggregateVal.size(); i++) {
      hash(HashThread, GV.AggregateVal[i]);
    }
  }
}
