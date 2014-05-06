//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef MM_UTILS
#define MM_UTILS

#include <stdlib.h>

inline size_t pointerDiff(void *Pointer1, void *Pointer2) {
  return static_cast<char*>(Pointer1) - static_cast<char*>(Pointer2);
}

inline void* pointerDiff(void *Pointer1, size_t Size) {
  return static_cast<char*>(Pointer1) - Size;
}

inline void* pointerAdd(void *Pointer1, size_t Size) {
  return static_cast<char*>(Pointer1) + Size;
}

#endif // MM_UTILS
