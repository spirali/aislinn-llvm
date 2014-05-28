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
#include "Action.h"

#include<vector>

namespace aislinn {

class Node;

class Arc
{
  public:

    Arc(Node *TargetNode, const std::vector<Action> &Actions) :
      TargetNode(TargetNode), Actions(Actions) {}
    Node *TargetNode;
    std::vector<Action> Actions;
};

class Node
{
  int Distance;
  Node *PrevNode;
  HashDigest Hash;
  std::vector<Arc> Arcs;

  public:
  Node(const HashDigest &Hash = HashDigest());

  const std::vector<Arc> & getArcs() const {
    return Arcs;
  }

  void addArc(const Arc &A) {
    updateDistance(A);
    Arcs.push_back(A);
  }

  HashDigest getHash() const {
    return Hash;
  }

  Node *getPrevNode() const {
    return PrevNode;
  }

  protected:
  void updateDistance(const Arc &A);
};

}

#endif // AISLINN_NODE
