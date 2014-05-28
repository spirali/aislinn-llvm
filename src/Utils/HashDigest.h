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
    Ptr = NULL;
  }

  explicit HashDigest(MHASH HashThread) {
    Ptr = mhash_end(HashThread);
  }

  HashDigest(const HashDigest &Digest) {
    if (Digest.isNull()) {
      Ptr = NULL;
      return;
    }
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
    if (this != &Other) {
      if (Other.isNull()) {
        if (!isNull()) {
          free(Ptr);
          Ptr = NULL;
        }
        return *this;
      }
      size_t Size = size();
      if (isNull()) {
        Ptr = malloc(Size);
      }
      memcpy(Ptr, Other.Ptr, Size);
    }
    return *this;
  }

  size_t size() const {
    return mhash_get_block_size(MHASH_MD5);
  }

  void toString(char *Out) const;

  bool isNull() const {
    return Ptr == NULL;
  }

  bool operator==(const HashDigest& Rhs) const {
    return memcmp(Ptr, Rhs.Ptr, size()) == 0;
  }

  bool operator< (const HashDigest& Rhs) const {
    return memcmp(Ptr, Rhs.Ptr, size()) < 0;
  }
};

}

#endif // AISLINN_HASH_DIGEST
