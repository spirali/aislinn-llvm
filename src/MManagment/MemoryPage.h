//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MEMORY_PAGE
#define AISLINN_MEMORY_PAGE

#include <stdio.h>
#include <stdlib.h>

namespace aislinn {

extern size_t PageSize;

class MemoryPage
{
  void *Data;
  bool Active;
  int RefCount;

  public:

  MemoryPage() {
    Data = NULL;
    RefCount = 1;
    Active = false;
  }

  ~MemoryPage() {
    if (Active) {
      fprintf(stderr, "Removing active page\n");
      exit(1);
    }
    if (Data) {
      free(Data);
    }
  }

  void setActive(size_t Value) {
    Active = Value;
  }

  bool isActive() const {
    return Active;
  }

  int getRefCount() const {
    return RefCount;
  }

  void incRefCount() {
    RefCount += 1;
  }

  void decRefCount() {
    RefCount -= 1;
    if (RefCount == 0) {
      delete this;
    }
  }

  bool isPrivate() const {
    return RefCount == 1;
  }

  void allocIfNeeded() {
    if (Data == NULL) {
      Data = malloc(PageSize);
    }
  }

  void *getData() {
    return Data;
  }

  void dump() {
    fprintf(stderr, "Page=%p RefCount=%i Active=%i Data=%p\n", this, RefCount, Active, Data);
  }

};

}

#endif // AISLINN_MEMORY_PAGE
