//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SeparateDeclarations function processes declarations, so that in the end each declaration
// contains only one declarator.
// This is useful as an intermediate step when initialization needs to be separated from
// declaration, or when things need to be unfolded out of the initializer.
// Example:
//     int a[1] = int[1](1), b[1] = int[1](2);
// gets transformed when run through this class into the AST equivalent of:
//     int a[1] = int[1](1);
//     int b[1] = int[1](2);

#include "compiler/translator/tree_ops/SeparateDeclarations.h"

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class SeparateDeclarationsTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root);

  private:
    SeparateDeclarationsTraverser();
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;
};

void SeparateDeclarationsTraverser::apply(TIntermNode *root)
{
    SeparateDeclarationsTraverser separateDecl;
    root->traverse(&separateDecl);
    separateDecl.updateTree();
}

SeparateDeclarationsTraverser::SeparateDeclarationsTraverser()
    : TIntermTraverser(true, false, false)
{
}

bool SeparateDeclarationsTraverser::visitDeclaration(Visit, TIntermDeclaration *node)
{
    TIntermSequence *sequence = node->getSequence();
    if (sequence->size() > 1)
    {
        TIntermBlock *parentBlock = getParentNode()->getAsBlock();
        ASSERT(parentBlock != nullptr);

        TIntermSequence replacementDeclarations;
        for (size_t ii = 0; ii < sequence->size(); ++ii)
        {
            TIntermDeclaration *replacementDeclaration = new TIntermDeclaration();

            replacementDeclaration->appendDeclarator(sequence->at(ii)->getAsTyped());
            replacementDeclaration->setLine(sequence->at(ii)->getLine());
            replacementDeclarations.push_back(replacementDeclaration);
        }

        mMultiReplacements.push_back(
            NodeReplaceWithMultipleEntry(parentBlock, node, replacementDeclarations));
    }
    return false;
}

}  // namespace

void SeparateDeclarations(TIntermNode *root)
{
    SeparateDeclarationsTraverser::apply(root);
}

}  // namespace sh
