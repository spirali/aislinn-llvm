//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#ifndef AISLINN_NODE
#define AISLINN_NODE

#include "../Utils/HashDigest.h"

#include<string>
#include<vector>

namespace aislinn {

class Node;

class Arc
{
  public:
    Arc(const std::string &Message, Node *TargetNode) :
      Message(Message), TargetNode(TargetNode) {}
    std::string Message;
    Node *TargetNode;
};

class Node
{
  HashDigest Hash;
  std::vector<Arc> Arcs;

  public:
  Node(const HashDigest &Hash);

  const std::vector<Arc> & getArcs() const {
    return Arcs;
  }

  void addArc(const Arc &A) {
    Arcs.push_back(A);
  }

  HashDigest getHash() const {
    return Hash;
  }
};

}

#endif // AISLINN_NODE
