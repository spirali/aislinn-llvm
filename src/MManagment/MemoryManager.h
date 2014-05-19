//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_MEMORY_MANAGER
#define AISLINN_MEMORY_MANAGER

#include "MemoryMapping.h"
#include "../Utils/Hash.h"

#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

namespace aislinn {

class MemoryManager {
  void *AddressSpace;
  size_t AddressSpaceSize;
  std::vector<ActivePage> ActivePages;
  MemoryMapping *Mapping;

  public:
    static void init(size_t AddressSpaceSize);

    MemoryManager(size_t AddressSpaceSize);
    ~MemoryManager();

    void dump();
    void setMapping(MemoryMapping *Mapping);

    void* allocPage();
    void swapPage(size_t Index); // Leaves page with protections READ, EXEC and WRITE
    void setProtectionToAll(int Protection);
    void fillPagesUpToIndex(int Index);
    void pageFaultOfAddress(void *Address);

    void setProtection(size_t Index, int Protection) {
      ActivePages[Index].Protection = Protection;
      mprotect(static_cast<char*>(AddressSpace) + Index * PageSize, PageSize, Protection);
    }

    size_t getPageOfAddress(void *Address) {
      size_t Index = ((char*) Address) - ((char*) AddressSpace);
      return Index /= PageSize;
    }

    void* getAddressOfPage(int Index) {
      return static_cast<char*>(AddressSpace) + Index * PageSize;
    }

    bool isInAddressSpace(void *Address) {
      return (Address > AddressSpace && \
          ((char*)Address - (char*)AddressSpace) < AddressSpaceSize);
    }

    void unmapPage(size_t Index) {
	if (Index < ActivePages.size()) {
		ActivePages[Index].Page->setActive(false);
		ActivePages[Index].Page = NULL;
		setProtection(Index, PROT_NONE);
	}
    }

    void *getBeginOfAddressSpace() {
      return AddressSpace;
    }

    void *getBreak() {
      return static_cast<char*>(AddressSpace) +
        Mapping->getPagesCount() * PageSize;
    }

  protected:

    void setPageAsActive(int Index, MemoryPage *Page) {
      if (ActivePages[Index].Page != NULL) {
        ActivePages[Index].Page->setActive(false);
      }
      ActivePages[Index].Page = Page;
      Page->setActive(true);
    }
};

extern MemoryManager *TheMemoryManager;
extern size_t PageSize;

}

#endif // AISLINN_MEMORY_MANAGER
