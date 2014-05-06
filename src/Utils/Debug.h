//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_DEBUG
#define AISLINN_DEBUG

#include "llvm/Support/raw_ostream.h"

#define IFVERBOSE(x) if (VerbosityLevel >= (x))

namespace aislinn {

extern unsigned int VerbosityLevel;

}

#endif
