//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "MemoryManager.h"
#include <stdio.h>

using namespace aislinn;

MemoryManager *aislinn::TheMemoryManager = NULL;
size_t aislinn::PageSize = 0;

MemoryManager::MemoryManager(size_t AddressSpaceSize) :
  AddressSpaceSize(AddressSpaceSize)
{
  AddressSpace = mmap(NULL,
                      AddressSpaceSize,
                      PROT_NONE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
  if (AddressSpace == MAP_FAILED) {
    perror("MemoryManager initialization");
    exit(1);
  }
}

MemoryManager::~MemoryManager()
{
  if (!munmap(AddressSpace, AddressSpaceSize)) {
    perror("MemoryManager descructor");
    exit(1);
  }
}

void MemoryManager::dump() {
  fprintf(stderr, "====== Memory manager =====\n");
  fprintf(stderr, "ActivePages = %lu\n", ActivePages.size());
  for (size_t I = 0; I < ActivePages.size(); I++) {
    fprintf(stderr,
        "Page Real=%p Index=%lu Page=%p Page.Data=%p - ",
        getAddressOfPage(I),
        I,
        ActivePages[I].Page,
        ActivePages[I].Page?ActivePages[I].Page->getData():NULL);
    if (ActivePages[I].Protection == PROT_NONE) {
      fprintf(stderr, "N");
    }
    if (ActivePages[I].Protection & PROT_READ) {
      fprintf(stderr, "R");
    }
    if (ActivePages[I].Protection & PROT_WRITE) {
      fprintf(stderr, "W");
    }
    if (ActivePages[I].Protection & PROT_EXEC) {
      fprintf(stderr, "E");
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "===============================\n");
}

void MemoryManager::setMapping(MemoryMapping *Mapping) {
  this->Mapping = Mapping;
  Mapping->setMemoryManager(this);
  setProtectionToAll(PROT_NONE);
}

void* MemoryManager::allocPage() {
  size_t Index = Mapping->allocPage();
  if (Index >= AddressSpaceSize / PageSize) {
    // TODO: Implement variant that signals error and do not exit
    fprintf(stderr, "Address space is full. "
                    "Application allocate to much memory. "
                    "The limit can be adjusted through --address-space-size");
    exit(1);
  }
  fillPagesUpToIndex(Index);
  swapPage(Index);
  MemoryPage *Page = Mapping->getPage(Index);
  setPageAsActive(Index, Page);
  void *Addr = getAddressOfPage(Index);
  return Addr;
}

// Leaves page with protections READ, EXEC and WRITE
void MemoryManager::swapPage(size_t Index) {
  setProtection(Index, PROT_READ | PROT_EXEC | PROT_WRITE);
  MemoryPage *Page = ActivePages[Index].Page;
  if (Page != NULL) {
    Page->allocIfNeeded();
    memcpy(Page->getData(), getAddressOfPage(Index), PageSize);
  }
}

void MemoryManager::setProtectionToAll(int Protection) {
  for (int i = 0; i < ActivePages.size(); i++) {
    ActivePages[i].Protection = Protection;
  }
  mprotect(AddressSpace, ActivePages.size() * PageSize, Protection);
}

void MemoryManager::fillPagesUpToIndex(int Index) {
  if (Index >= ActivePages.size()) {
    ActivePage AP;
    AP.Page = NULL;
    AP.Protection = PROT_NONE;
    for (int I = ActivePages.size(); I <= Index; I++) {
      ActivePages.push_back(AP);
    }
  }
}

void MemoryManager::pageFaultOfAddress(void *Address) {
  size_t Index = getPageOfAddress(Address);
  MemoryPage *Page = Mapping->getPage(Index);
  if (Page == NULL) {
    fprintf(stderr, "Acces to unmapped memory");
    exit(1);
  }
  fillPagesUpToIndex(Index);

  ActivePage &AP = ActivePages[Index];
  if (ActivePages[Index].Protection == PROT_NONE) {
    if (Page == AP.Page) {
      int Protection = PROT_READ | PROT_EXEC;
      if (Page->isPrivate()) {
        Protection |= PROT_WRITE;
      }
      setProtection(Index, Protection);
      return;
    }
    setProtection(Index, PROT_WRITE | PROT_READ | PROT_EXEC);
    swapPage(Index);
    memcpy(getAddressOfPage(Index), Page->getData(), PageSize);
    setPageAsActive(Index, Page);
    if (!Page->isPrivate()) {
      setProtection(Index, PROT_READ | PROT_EXEC);
    }
  } else {
    // Writing into read only page, i.e. page is shared so we
    // have to make a private copy
    swapPage(Index);
    MemoryPage *Page = Mapping->emptyPage(Index);
    setPageAsActive(Index, Page);
    setProtection(Index, PROT_WRITE | PROT_READ | PROT_EXEC);
  }
}

static void signal_handler(int sig, siginfo_t *si, void *unused)
{
  char *mem = (char*) si->si_addr;
  if (TheMemoryManager && TheMemoryManager->isInAddressSpace(mem)) {
    //fprintf(stderr, "Got SIGSEGV at address: %p %lu\n", mem, TheMemoryManager->getPageOfAddress(mem));
    TheMemoryManager->pageFaultOfAddress(mem);
    return;
  }
  //fprintf(stderr, "Got SIGSEGV at address: %p\n", mem);
  const char *msg = "Segmentation fault\n";
  write(2, msg, strlen(msg));
  exit(1);
}

void MemoryManager::init(size_t AddressSpaceSize)
{
  if (TheMemoryManager != NULL) {
    fprintf(stderr, "Memory manager is already initialized\n");
    exit(1);
  }
  PageSize = getpagesize();

  if (AddressSpaceSize == 0) { // Use default
    if (sizeof(void*) == 4) {
      AddressSpaceSize = 128 * 1024 * 1024; // 128MB on 32b architecture
    } else {
      AddressSpaceSize = 1024 * 1024 * 1024; // 1GB on 64b architecture
    }
  }

  if (AddressSpaceSize % PageSize != 0) {
    fprintf(stderr, "Size of address space is not multiple of page size\n");
    exit(1);
  }

  TheMemoryManager = new MemoryManager(AddressSpaceSize);

  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = signal_handler;
  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
}


