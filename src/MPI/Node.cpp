//===-----------------------------------------------------------*- C++ -*--===//
//
//                     Aislinn
//
// This file is distributed under the University of Illinois Open Source
// License. See COPYING for details.
//
//===----------------------------------------------------------------------===//

#include "Node.h"

using namespace aislinn;

Node::Node(const HashDigest &Hash) :
  PrevNode(NULL), Distance(0), Hash(Hash)
{
}

void Node::updateDistance(const Arc &A)
{
  int D = Distance + A.Actions.size();
  if (A.TargetNode->PrevNode == NULL || D < A.TargetNode->Distance) {
    A.TargetNode->PrevNode = this;
    A.TargetNode->Distance = D;
  }
}
