//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "MemoryObjects.h"
#include "MemoryMapping.h"
#include "MemoryManager.h"
#include "Utils.h"

using namespace aislinn;

MemoryObjects::MemoryObjects()
{
  Mapping = new MemoryMapping;
}

MemoryObjects::MemoryObjects(const MemoryObjects &MO)
	: Allocations(MO.Allocations)
{
	Mapping = new MemoryMapping(MO.Mapping);
}

MemoryObjects::~MemoryObjects()
{
  delete Mapping;
}

void MemoryObjects::makeActive()
{
  TheMemoryManager->setMapping(Mapping);
}

void* MemoryObjects::alloc(size_t Size)
{
  if (Size == 0) {
    Size = 1;
  }
  // Allign
  Size = (((Size - 1) / sizeof(void*)) + 1) * sizeof(void*);
  if (Allocations.size() == 0) {
    char *NewPage = static_cast<char*>(TheMemoryManager->allocPage());
    Allocation A;
    A.Pointer = NewPage;
    A.Type = ALLOCATION_FREE;
    Allocations.push_back(A);
    A.Pointer = NewPage + PageSize;
    A.Type = ALLOCATION_END;
    Allocations.push_back(A);
  }

  for (size_t i = 0; i < Allocations.size() - 1; i++) {
    Allocation &A = Allocations[i];
    if (A.Type == ALLOCATION_FREE) {
      size_t S = pointerDiff(Allocations[i + 1].Pointer,\
                             Allocations[i].Pointer);
      if (S == Size) {
        A.Type = ALLOCATION_USED;
        return A.Pointer;
      }

      if (S >= Size) {
	// We are going to change Allocations, so A will be invalid
	void *P = A.Pointer;
        A.Type = ALLOCATION_USED;
        Allocation A2;
        A2.Pointer = pointerAdd(A.Pointer, Size);
        A2.Type = ALLOCATION_FREE;
        Allocations.insert(Allocations.begin() + i + 1, A2);
        return P;
      }
    }
  }

  size_t i = Allocations.size() - 2;
  if (Allocations[i].Type != ALLOCATION_FREE) {
    i++;
    moveEnd(Allocations[i].Pointer, ALLOCATION_USED);
  } else {
    Allocations[i].Type = ALLOCATION_USED;
  }

  size_t S = pointerDiff(Allocations[i + 1].Pointer, Allocations[i].Pointer);
  size_t NeedPages = (Size - S - 1) / PageSize + 1;

  for (int j = 0; j < NeedPages; j++) {
    // TODO: Implement allocation of more pages at once
    TheMemoryManager->allocPage();
  }

  Allocations[i+1].Pointer = pointerAdd(Allocations[i].Pointer, Size);

  void *Break = TheMemoryManager->getBreak();
  if (pointerDiff(Break, Allocations[i+1].Pointer)) {
    moveEnd(Break, ALLOCATION_FREE);
  }
  return Allocations[i].Pointer;
}

void MemoryObjects::free(void *Pointer)
{
  for (size_t i = 0; i < Allocations.size(); i++) {
    if (Allocations[i].Pointer == Pointer) {
      Allocations[i].Type = ALLOCATION_FREE;
      if (i < Allocations.size() - 1 &&
          Allocations[i + 1].Type == ALLOCATION_FREE) {
        Allocations.erase(Allocations.begin() + i + 1);
      }
      if (i > 0 && Allocations[i-1].Type == ALLOCATION_FREE) {
        Allocations.erase(Allocations.begin() + i);
      }
      return;
    }
  }
  fprintf(stderr, "Invalid free %p\n", Pointer);
  exit(1);
}

void MemoryObjects::dump()
{
  Mapping->dump();
  fprintf(stderr, "MEMORY OBJECTS %p\n", this);
  for (size_t i = 0; i < Allocations.size(); i++) {
    char c;
    switch (Allocations[i].Type) {
      case ALLOCATION_FREE : c = 'f'; break;
      case ALLOCATION_USED : c = 'u'; break;
      case ALLOCATION_END : c = 'e'; break;
      default : c = 'e'; break;
    }
    fprintf(stderr, "Index=%lu Pointer=%p Type=%c\n", i, Allocations[i].Pointer, c);
  }
}

void MemoryObjects::hash(MHASH HashThread)
{
  for (size_t i = 0; i < Allocations.size(); i++) {
    if (Allocations[i].Type == ALLOCATION_USED) {
      void *Addr = Allocations[i].Pointer;
      size_t Size = pointerDiff(Allocations[i + 1].Pointer, Addr);
      mhash(HashThread, &Size, sizeof(Size));
      Mapping->hashMemory(HashThread, Addr, Size);
    }
  }
}
