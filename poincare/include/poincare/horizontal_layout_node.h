#ifndef POINCARE_HORIZONTAL_LAYOUT_NODE_H
#define POINCARE_HORIZONTAL_LAYOUT_NODE_H

#include "layout_reference.h"
#include "layout_node.h"
#include "layout_cursor.h"

namespace Poincare {

/* WARNING: A HorizontalLayout should never have a HorizontalLayout child. For
 * instance, use addOrMergeChildAtIndex to add a LayoutNode safely. */

class HorizontalLayoutNode : public LayoutNode {
public:
  HorizontalLayoutNode() :
    LayoutNode(),
    m_numberOfChildren(0)
  {}

  // Tree navigation
  LayoutCursor equivalentCursor(LayoutCursor * cursor) override;

  // Tree modification
  void deleteBeforeCursor(LayoutCursor * cursor) override;

  // SerializableNode
  int writeTextInBuffer(char * buffer, int bufferSize, int numberOfSignificantDigits = PrintFloat::k_numberOfStoredSignificantDigits) const override;

  // LayoutNode
  bool isHorizontal() const override { return true; }
  void moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) override;
  void moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) override;

  bool willAddSibling(LayoutCursor * cursor, LayoutNode * sibling, bool moveCursor) override;
  bool willRemoveChild(LayoutNode * l, LayoutCursor * cursor) override;
  void didRemoveChildAtIndex(int index, LayoutCursor * cursor, bool force) override;
  bool willReplaceChild(LayoutNode * oldChild, LayoutNode * newChild, LayoutCursor * cursor) override;

  // TreeNode
  size_t size() const override { return sizeof(HorizontalLayoutNode); }
  int numberOfChildren() const override { return m_numberOfChildren; }
  void incrementNumberOfChildren(int increment = 1) override { m_numberOfChildren+= increment; }
  void decrementNumberOfChildren(int decrement = 1) override {
    assert(m_numberOfChildren >= decrement);
    m_numberOfChildren-= decrement;
  }
  void eraseNumberOfChildren() override { m_numberOfChildren = 0; }
#if TREE_LOG
  const char * description() const override {
    return "Horizontal Layout";
  }
#endif

protected:
  // LayoutNode
  void computeSize() override;
  void computeBaseline() override;
  KDPoint positionOfChild(LayoutNode * l) override;

private:
  void render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) override {}
  int m_numberOfChildren;
};

class HorizontalLayoutRef : public LayoutReference<HorizontalLayoutNode> {
public:
  HorizontalLayoutRef() : LayoutReference<HorizontalLayoutNode>() {}
  HorizontalLayoutRef(TreeNode * hNode) : LayoutReference<HorizontalLayoutNode>(hNode) {}
  HorizontalLayoutRef(LayoutRef l) : LayoutReference<HorizontalLayoutNode>() {
    addChildTreeAtIndex(l, 0);
  }

  HorizontalLayoutRef(LayoutRef l1, LayoutRef l2) : LayoutReference<HorizontalLayoutNode>() {
    addChildTreeAtIndex(l1, 0);
    addChildTreeAtIndex(l2, 1);
  }

  void addOrMergeChildAtIndex(LayoutRef l, int index, bool removeEmptyChildren, LayoutCursor * cursor = nullptr);
  void mergeChildrenAtIndex(HorizontalLayoutRef h, int index, bool removeEmptyChildren, LayoutCursor * cursor = nullptr);
private:
  int removeEmptyChildBeforeInsertionAtIndex(int index, bool shouldRemoveOnLeft);
};

}

#endif
