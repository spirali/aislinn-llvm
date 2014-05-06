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
/// \brief Reference counting classes. Based on KLEE code.
///
//===----------------------------------------------------------------------===//

#ifndef KLEE_Ref_H
#define KLEE_Ref_H

#include "llvm/Support/Casting.h"
using llvm::isa;
using llvm::cast;
using llvm::cast_or_null;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;

#include <assert.h>

namespace aislinn {

template<class T>
class Ref {
  T *Ptr;

public:
  // default constructor: create a NULL Reference
  Ref() : Ptr(0) { }
  ~Ref () { dec (); }

private:
  void inc() const {
    if (Ptr)
      ++Ptr->RefCount;
  }

  void dec() const {
    if (Ptr && --Ptr->RefCount == 0)
      delete Ptr;
  }

public:
  template<class U> friend class Ref;

  // constructor from pointer
  Ref(T *p) : Ptr(p) {
    inc();
  }

  // normal copy constructor
  Ref(const Ref<T> &r) : Ptr(r.Ptr) {
    inc();
  }

  // conversion constructor
  template<class U>
  Ref (const Ref<U> &r) : Ptr(r.Ptr) {
    inc();
  }

  // pointer operations
  T *get () const {
    return Ptr;
  }

  /* The copy assignment operator must also explicitly be defined,
   * despite a redundant template. */
  Ref<T> &operator= (const Ref<T> &r) {
    r.inc();
    dec();
    Ptr = r.Ptr;

    return *this;
  }

  template<class U> Ref<T> &operator= (const Ref<U> &r) {
    r.inc();
    dec();
    Ptr = r.Ptr;

    return *this;
  }

  T& operator*() const {
    return *Ptr;
  }

  T* operator->() const {
    return Ptr;
  }

  bool isNull() const {
    return Ptr == 0;
  }

  // assumes non-null arguments
  int compare(const Ref &rhs) const {
    assert(!isNull() && !rhs.isNull() && "Invalid call to compare()");
    return get()->compare(*rhs.get());
  }

  // assumes non-null arguments
  bool operator<(const Ref &rhs) const {
    return compare(rhs) < 0;
  }

  bool operator==(const Ref &rhs) const {
    return compare(rhs) == 0;
  }

  bool operator!=(const Ref &rhs) const {
    return compare(rhs) != 0;
  }

};

class RefCountedObj {
  public:
    RefCountedObj() : RefCount(0) {}
    mutable unsigned RefCount;
};

} // end namespace

namespace llvm {
  // simplify_type implementation for Ref<>, which allows dyn_cast from on a
  // Ref<> to apply to the wrapper type. Conceptually the result of such a
  // dyn_cast should probably be a Ref of the casted type, but that breaks the
  // idiom of initializing a variable to the result of a dyn_cast inside an if
  // condition, or we would have to implement operator(bool) for Ref<> with
  // isNull semantics, which doesn't seem like a good idea.
template<typename T>
struct simplify_type<const ::aislinn::Ref<T> > {
  typedef T* SimpleType;
  static SimpleType getSimplifiedValue(const ::aislinn::Ref<T> &Ref) {
    return Ref.get();
  }
};

template<typename T>
struct simplify_type< ::aislinn::Ref<T> >
  : public simplify_type<const ::aislinn::Ref<T> > {};
}

#endif
