//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MEMORY_MAPPING
#define AISLINN_MEMORY_MAPPING

#include "MemoryPage.h"
#include "../Utils/Hash.h"
#include <vector>

namespace aislinn {

class MemoryManager;

struct ActivePage {
  MemoryPage *Page;
  int Protection;
};

class MemoryMapping {
  std::vector<MemoryPage*> Pages;
  MemoryManager *Manager;

  public:
  MemoryMapping();
  MemoryMapping(MemoryMapping *Mapping);
  ~MemoryMapping();

  void setMemoryManager(MemoryManager *MM) {
    Manager = MM;
  }

  MemoryPage* getPage(size_t Index) {
    if (Index < Pages.size()) {
      return Pages[Index];
    }
    return NULL;
  }

  MemoryPage* emptyPage(size_t Index) {
    Pages[Index]->decRefCount();
    MemoryPage *Page = new MemoryPage();
    Pages[Index] = Page;
    return Page;
  }

  // Usually you want to call MemoryManager::allocPage(), not directly this
  size_t allocPage() {
    Pages.push_back(new MemoryPage());
    return Pages.size() - 1;
  }

  size_t getPagesCount() {
    return Pages.size();
  }

  void hashMemory(MHASH HashThread, void *Addr, size_t Size);
  void dump();

};

}

#endif
