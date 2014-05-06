//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "MemoryMapping.h"
#include "MemoryManager.h"
#include "Utils.h"

using namespace aislinn;

MemoryMapping::MemoryMapping() {
  Manager = NULL;
}

MemoryMapping::MemoryMapping(MemoryMapping *Mapping) : Pages(Mapping->Pages) {
  Manager = Mapping->Manager;
  bool InUse = false;
  for (int I = 0; I < Pages.size(); I++) {
    Pages[I]->incRefCount();
    InUse |= Pages[I]->isActive();
  }
  if (InUse && Manager) {
    Manager->setProtectionToAll(PROT_NONE);
  }
}

MemoryMapping::~MemoryMapping()
{
  for (int I = 0; I < Pages.size(); I++) {
    if (Pages[I]->isPrivate() && Pages[I]->isActive() && Manager) {
      Manager->unmapPage(I);
    }
    Pages[I]->decRefCount();
  }
}

void MemoryMapping::dump() {
  fprintf(stderr, "------- MemoryMapping ---- \n");
  for (size_t i = 0; i < Pages.size(); i++) {
    fprintf(stderr, "%lu: ", i);
    Pages[i]->dump();
  }
  fprintf(stderr, "-------------------------- \n");
}

void MemoryMapping::hashMemory(MHASH HashThread, void *Addr, size_t Size)
{
  assert(Manager != NULL);
  size_t P = Manager->getPageOfAddress(Addr);
  void *E = Manager->getAddressOfPage(P + 1);
  MemoryPage *PG = getPage(P);
  size_t S = pointerDiff(E, Addr);

  for (;;) {
    if (S >= Size) {
      S = Size;
    }

    if (PG->isActive()) {
        mhash(HashThread, Addr, S);
    } else {
      void *B = pointerDiff(E, PageSize);
      size_t Shift = pointerDiff(Addr, B);
      mhash(HashThread, pointerAdd(PG->getData(), Shift), S);
    }

    Size -= S;

    if (Size == 0) {
      return;
    }

    Addr = E;
    E = pointerAdd(E, PageSize);
    S = PageSize;
  }
}
