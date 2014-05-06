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
/// \brief The class representings hash digest
///
//===----------------------------------------------------------------------===//

#ifndef AISLINN_HASH_DIGEST
#define AISLINN_HASH_DIGEST

#include <mhash.h>

#define ALLOCA_STRING_FOR_HASH \
 ((char*) alloca(mhash_get_block_size(MHASH_MD5) * 2 + 1))

namespace aislinn {

class HashDigest {
  void *Ptr;

  public:
  HashDigest() {
    Ptr = malloc(size());
    memset(Ptr, 0, size());
  }

  explicit HashDigest(MHASH HashThread) {
    Ptr = mhash_end(HashThread);
  }

  HashDigest(const HashDigest &Digest) {
    size_t Size = size();
    Ptr = malloc(Size);
    memcpy(Ptr, Digest.Ptr, Size);
  }

  ~HashDigest() {
    if (Ptr) {
      free(Ptr);
    }
  }

  HashDigest& operator=(const HashDigest& Other)
  {
      memcpy(Ptr, Other.Ptr, size());
      return *this;
  }

  size_t size() const {
    return mhash_get_block_size(MHASH_MD5);
  }

  void toString(char *Out) const;

  bool operator==(const HashDigest& Rhs) const {
    return memcmp(Ptr, Rhs.Ptr, size()) == 0;
  }

  bool operator< (const HashDigest& Rhs) const {
    return memcmp(Ptr, Rhs.Ptr, size()) < 0;
  }
};

}

#endif // AISLINN_HASH_DIGEST
