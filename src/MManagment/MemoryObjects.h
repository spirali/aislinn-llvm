//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MEMORY_OBJECTS
#define AISLINN_MEMORY_OBJECTS

#include "../Utils/Hash.h"
#include <vector>
#include <stdlib.h>

namespace aislinn {

class MemoryMapping;

class MemoryObjects
{
  enum AllocationType {
    ALLOCATION_FREE,
    ALLOCATION_USED,
    ALLOCATION_END
  };

  struct Allocation {
    void *Pointer;
    AllocationType Type;
  };

  MemoryMapping *Mapping;
  std::vector<Allocation> Allocations;

  public:
  MemoryObjects();
  MemoryObjects(const MemoryObjects &MO);
  ~MemoryObjects();

  void makeActive();

  void *alloc(size_t Size);
  void free(void *Pointer);
  void dump();
  void hash(MHASH HashThread);

  protected:

  void moveEnd(void *Pointer, AllocationType Type) {
    size_t i = Allocations.size() - 1;
    Allocations[i].Type = Type;
    Allocation A;
    A.Pointer = Pointer;
    A.Type = ALLOCATION_END;
    Allocations.push_back(A);
  }
};

}

#endif // AISLINN_MEMORY_OBJECTS
