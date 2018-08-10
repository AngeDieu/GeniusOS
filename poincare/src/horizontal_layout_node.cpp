#include <poincare/horizontal_layout_node.h>
#include <poincare/allocation_failure_layout_node.h>
#include <poincare/empty_layout_node.h>
#include <poincare/layout_helper.h>
#include <poincare/serialization_helper.h>

namespace Poincare {

static inline KDCoordinate maxCoordinate(KDCoordinate c1, KDCoordinate c2) { return c1 > c2 ? c1 : c2; }

HorizontalLayoutNode * HorizontalLayoutNode::FailedAllocationStaticNode() {
  static AllocationFailureLayoutNode<HorizontalLayoutNode> failure;
  TreePool::sharedPool()->registerStaticNodeIfRequired(&failure);
  return &failure;
}

// LayoutNode

void HorizontalLayoutNode::moveCursorLeft(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (this == cursor->layoutNode()) {
    if (cursor->position() == LayoutCursor::Position::Left) {
      // Case: Left. Ask the parent.
      LayoutNode * parentNode = parent();
      if (parentNode != nullptr) {
        parentNode->moveCursorLeft(cursor, shouldRecomputeLayout);
      }
      return;
    }
    assert(cursor->position() == LayoutCursor::Position::Right);
    /* Case: Right. Go to the last child if there is one, and move Left. Else
     * go Left and ask the parent. */
    int childrenCount = numberOfChildren();
    if (childrenCount >= 1) {
      cursor->setLayoutNode(childAtIndex(childrenCount-1));
    } else {
      cursor->setPosition(LayoutCursor::Position::Left);
    }
    return cursor->moveLeft(shouldRecomputeLayout);
  }

  // Case: The cursor is Left of a child.
  assert(cursor->position() == LayoutCursor::Position::Left);
  int childIndex = indexOfChild(cursor->layoutNode());
  assert(childIndex >= 0);
  if (childIndex == 0) {
    // Case: the child is the leftmost. Ask the parent.
    if (parent()) {
      cursor->setLayoutNode(this);
      return cursor->moveLeft(shouldRecomputeLayout);
    }
    return;
  }
  // Case: the child is not the leftmost. Go to its left sibling and move Left.
  cursor->setLayoutNode(childAtIndex(childIndex-1));
  cursor->setPosition(LayoutCursor::Position::Right);
  cursor->moveLeft(shouldRecomputeLayout);
}

void HorizontalLayoutNode::moveCursorRight(LayoutCursor * cursor, bool * shouldRecomputeLayout) {
  if (this == cursor->layoutNode()) {
    if (cursor->position() == LayoutCursor::Position::Right) {
      // Case: Right. Ask the parent.
      LayoutNode * parentNode = parent();
      if (parentNode) {
        parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
      }
      return;
    }
    assert(cursor->position() == LayoutCursor::Position::Left);
    // Case: Left
    int childrenCount = numberOfChildren();
    LayoutNode * parentNode = parent();
    if (childrenCount == 0) {
      // If there are no children, go Right and ask the parent
      cursor->setPosition(LayoutCursor::Position::Right);
      if (parentNode != nullptr) {
        parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
      }
      return;
    }
    /* If there is at least one child, set the cursor to the first child and
     * move it Right */
    LayoutNode * firstChild = childAtIndex(0);
    assert(firstChild != nullptr);
    cursor->setLayoutNode(firstChild);
    return firstChild->moveCursorRight(cursor, shouldRecomputeLayout);
  }

  // Case: The cursor is Right of a child.
  assert(cursor->position() == LayoutCursor::Position::Right);
  int childIndex = indexOfChild(cursor->layoutNode());
  assert(childIndex >= 0);
  if (childIndex == numberOfChildren() - 1) {
    // Case: the child is the rightmost. Ask the parent.
    LayoutNode * parentNode = parent();
    if (parentNode) {
      cursor->setLayoutNode(this);
      parentNode->moveCursorRight(cursor, shouldRecomputeLayout);
    }
    return;
  }
  /* Case: the child is not the rightmost. Go to its right sibling and move
   * Right. */
  cursor->setLayoutNode(childAtIndex(childIndex+1));
  cursor->setPosition(LayoutCursor::Position::Left);
  return childAtIndex(childIndex+1)->moveCursorRight(cursor, shouldRecomputeLayout);
}

LayoutCursor HorizontalLayoutNode::equivalentCursor(LayoutCursor * cursor) {
  if (cursor->layoutNode() == this) {
    // First or last child, if any
    int childrenCount = numberOfChildren();
    if (childrenCount == 0) {
      return LayoutCursor();
    }
    int index = cursor->position() == LayoutCursor::Position::Left ? 0 : childrenCount - 1;
    return LayoutCursor(childAtIndex(index), cursor->position());
  }
  // Left or right of a child: return right or left of its sibling, or of this
  int indexOfPointedLayout = indexOfChild(cursor->layoutNode());
  if (indexOfPointedLayout < 0) {
    return LayoutCursor();
  }
  if (cursor->position() == LayoutCursor::Position::Left) {
    if (indexOfPointedLayout == 0) {
      return LayoutCursor(this, LayoutCursor::Position::Left);
    }
    return LayoutCursor(childAtIndex(indexOfPointedLayout - 1), LayoutCursor::Position::Right);
  }
  assert(cursor->position() == LayoutCursor::Position::Right);
  if (indexOfPointedLayout == numberOfChildren() - 1) {
    return LayoutCursor(this, LayoutCursor::Position::Right);
  }
  return LayoutCursor(childAtIndex(indexOfPointedLayout + 1), LayoutCursor::Position::Left);
}

void HorizontalLayoutNode::deleteBeforeCursor(LayoutCursor * cursor) {
  LayoutNode * p = parent();
  if (p == nullptr
      && cursor->layoutNode() == this
      && (cursor->position() == LayoutCursor::Position::Left
        || numberOfChildren() == 0))
  {
    /* Case: Left and this is the main layout or Right and this is the main
     * layout with no children. Return. */
    return;
  }
  if (cursor->position() == LayoutCursor::Position::Left) {
    int indexOfPointedLayout = indexOfChild(cursor->layoutNode());
    if (indexOfPointedLayout >= 0) {
      /* Case: Left of a child.
       * Point Right of the previous child. If there is no previous child, point
       * Left of this. Perform another backspace. */
      if (indexOfPointedLayout == 0) {
        cursor->setLayoutNode(this);
      } else {
        assert(indexOfPointedLayout > 0);
        cursor->setLayoutNode(childAtIndex(indexOfPointedLayout - 1));
        cursor->setPosition(LayoutCursor::Position::Right);
      }
      cursor->performBackspace();
      return;
    }
  }
  assert(cursor->layoutNode() == this);
  if (cursor->position() == LayoutCursor::Position::Right) {
    // Case: Right. Point to the last child and perform backspace.
    cursor->setLayoutNode(childAtIndex(numberOfChildren() - 1));
    cursor->performBackspace();
    return;
  }
  LayoutNode::deleteBeforeCursor(cursor);
}

int HorizontalLayoutNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  if (numberOfChildren() == 0) {
    if (bufferSize == 0) {
      return -1;
    }
    buffer[0] = 0;
    return 0;
  }
  return SerializationHelper::Infix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, "");
}

bool HorizontalLayoutNode::hasText() const override {
  if (numberOfChildren() == 0 || ) {
    return false;
  }
  if (numberOfChildren() ==  1 && !(const_cast<HorizontalLayoutNode *>(this)->childAtIndex(0)->hasText())) {
    return false;
  }
  return true;
}

// Protected

KDSize HorizontalLayoutNode::computeSize() {
  KDCoordinate totalWidth = 0;
  KDCoordinate maxUnderBaseline = 0;
  KDCoordinate maxAboveBaseline = 0;
  for (LayoutNode * l : children()) {
    KDSize childSize = l->layoutSize();
    totalWidth += childSize.width();
    maxUnderBaseline = maxCoordinate(maxUnderBaseline, childSize.height() - l->baseline());
    maxAboveBaseline = maxCoordinate(maxAboveBaseline, l->baseline());
  }
  return KDSize(totalWidth, maxUnderBaseline + maxAboveBaseline);
}

KDCoordinate HorizontalLayoutNode::computeBaseline() {
  KDCoordinate result = 0;
  for (LayoutNode * l : children()) {
    result = maxCoordinate(result, l->baseline());
  }
  return result;
}

KDPoint HorizontalLayoutNode::positionOfChild(LayoutNode * l) {
  assert(hasChild(l));
  KDCoordinate x = 0;
  int index = indexOfChild(l);
  assert(index > -1);
  if (index > 0) {
    LayoutNode * previousChild = childAtIndex(index-1);
    x = previousChild->origin().x() + previousChild->layoutSize().width();
  }
  KDCoordinate y = baseline() - l->baseline();
  return KDPoint(x, y);
}

// Private

bool HorizontalLayoutNode::willAddChildAtIndex(LayoutNode * l, int * index, int * currentNumberOfChildren, LayoutCursor * cursor) {
  if (m_numberOfChildren > 0) {
    HorizontalLayoutRef(this).removeEmptyChildBeforeInsertionAtIndex(index, currentNumberOfChildren, !l->mustHaveLeftSibling(), cursor);
  }
  return true;
}

bool HorizontalLayoutNode::willAddSibling(LayoutCursor * cursor, LayoutNode * sibling, bool moveCursor) {
  HorizontalLayoutRef thisRef(this);
  int newChildIndex = cursor->position() == LayoutCursor::Position::Left ? 0 : numberOfChildren();
  thisRef.addOrMergeChildAtIndex(sibling, newChildIndex, true, cursor);
  return false;
}

bool HorizontalLayoutNode::willRemoveChild(LayoutNode * l, LayoutCursor * cursor, bool force) {
  if (!force && numberOfChildren() == 1) {
    assert(childAtIndex(0) == l);
    LayoutNode * p = parent();
    if (p != nullptr) {
      LayoutRef(p).removeChild(HorizontalLayoutRef(this), cursor);
      // WARNING: Do not call "this" afterwards
      return false;
    }
  }
  return true;
}

void HorizontalLayoutNode::didRemoveChildAtIndex(int index, LayoutCursor * cursor, bool force) {
  /* If the child to remove was at index 0 and its right sibling must have a left
   * sibling (e.g. a VerticalOffsetLayout), add an empty layout at index 0 */

  if (!force && index == 0 && numberOfChildren() > 0 && childAtIndex(0)->mustHaveLeftSibling()) {
    LayoutRef(this).addChildAtIndex(EmptyLayoutRef(), 0, numberOfChildren(), cursor);
  }
}

bool HorizontalLayoutNode::willReplaceChild(LayoutNode * oldChild, LayoutNode * newChild, LayoutCursor * cursor, bool force) {
  if (oldChild == newChild) {
    return false;
  }
  if (force) {
    return true;
  }
  HorizontalLayoutRef thisRef(this);
  int oldChildIndex = indexOfChild(oldChild);
  if (newChild->isEmpty()) {
    if (numberOfChildren() > 1) {
      /* If the new layout is empty and the horizontal layout has other
       * children, just remove the old child. */
      thisRef.removeChild(oldChild, nullptr);
      if (cursor != nullptr) {
        if (oldChildIndex == 0) {
          cursor->setLayoutReference(thisRef);
          cursor->setPosition(LayoutCursor::Position::Left);
        } else {
          cursor->setLayoutReference(thisRef.childAtIndex(oldChildIndex -1));
          cursor->setPosition(LayoutCursor::Position::Right);
        }
      }
      return false;
    }
    /* The old layout was the only horizontal layout child, so if this has a
     * a parent, replace this with the new empty layout. */
    LayoutNode * p = parent();
    if (p != nullptr) {
      thisRef.replaceWith(newChild, cursor);
      // WARNING: do not call "this" afterwards
      return false;
    }
    /* This is the main horizontal layout, the old child is its only child and
     * the new child is Empty: remove the old child and delete the new child. */
    assert(p == nullptr);
    thisRef.removeChild(oldChild, nullptr);
    // WARNING: do not call "this" afterwards
    if (cursor != nullptr) {
      cursor->setLayoutReference(thisRef);
      cursor->setPosition(LayoutCursor::Position::Left);
    }
    return false;
  }
  /* If the new child is also an horizontal layout, steal the children of the
   * new layout then destroy it. */
  bool oldWasAncestorOfNewLayout = newChild->hasAncestor(oldChild, false);
  if (newChild->isHorizontal()) {
    int indexForInsertion = indexOfChild(oldChild);
    if (cursor != nullptr) {
      /* If the old layout is not an ancestor of the new layout, or if the
       * cursor was on the right of the new layout, place the cursor on the
       * right of the new layout, which is left of the next sibling or right of
       * the parent. */
      if (!oldWasAncestorOfNewLayout || cursor->position() == LayoutCursor::Position::Right) {
        if (oldChildIndex == numberOfChildren() - 1) {
          cursor->setLayoutNode(this);
          cursor->setPosition(LayoutCursor::Position::Right);
        } else {
          cursor->setLayoutNode(childAtIndex(oldChildIndex + 1));
          cursor->setPosition(LayoutCursor::Position::Left);
        }
      } else {
        /* Else place the cursor on the left of the new layout, which is right
         * of the previous sibling or left of the parent. */
        if (oldChildIndex == 0) {
          cursor->setLayoutNode(this);
          cursor->setPosition(LayoutCursor::Position::Left);
        } else {
          cursor->setLayoutNode(childAtIndex(oldChildIndex - 1));
          cursor->setPosition(LayoutCursor::Position::Right);
        }
      }
    }
    bool oldChildRemovedAtMerge = oldChild->isEmpty();
    thisRef.mergeChildrenAtIndex(HorizontalLayoutRef(static_cast<HorizontalLayoutNode *>(newChild)), indexForInsertion + 1, true);
    // WARNING: do not call "this" afterwards
    if (!oldChildRemovedAtMerge) {
      thisRef.removeChildAtIndex(indexForInsertion, cursor);
    }
    return false;
  }
  // Else, just replace the child.
  if (cursor != nullptr && !oldWasAncestorOfNewLayout) {
    cursor->setPosition(LayoutCursor::Position::Right);
  }
  return true;
}

// HorizontalLayoutRef

void HorizontalLayoutRef::addOrMergeChildAtIndex(LayoutRef l, int index, bool removeEmptyChildren, LayoutCursor * cursor) {
  if (isAllocationFailure()) {
    return;
  }
  if (l.isHorizontal()) {
    mergeChildrenAtIndex(HorizontalLayoutRef(static_cast<HorizontalLayoutNode *>(l.node())), index, removeEmptyChildren, cursor);
  } else {
    addChildAtIndex(l, index, numberOfChildren(),cursor);
  }
}

void HorizontalLayoutRef::mergeChildrenAtIndex(HorizontalLayoutRef h, int index, bool removeEmptyChildren, LayoutCursor * cursor) {
  if (isAllocationFailure()) {
    return;
  }
  /* Remove any empty child that would be next to the inserted layout.
   * If the layout to insert starts with a vertical offset layout, any empty
   * layout child directly on the left of the inserted layout (if there is one)
   * should not be removed: it will be the base for the VerticalOffsetLayout. */
  bool shouldRemoveOnLeft = h.numberOfChildren() == 0 ? true : !(h.childAtIndex(0).mustHaveLeftSibling());
  int newIndex = index;
  removeEmptyChildBeforeInsertionAtIndex(&newIndex, nullptr, shouldRemoveOnLeft);

  LayoutRef nextPointedLayout(nullptr);
  LayoutCursor::Position nextPosition = LayoutCursor::Position::Left;
  if (newIndex < numberOfChildren()) {
    nextPointedLayout = childAtIndex(newIndex);
    nextPosition = LayoutCursor::Position::Left;
  } else {
    nextPointedLayout = *this;
    nextPosition = LayoutCursor::Position::Right;
  }

  // Merge the horizontal layout
  mergeChildrenAtIndexInPlace(h, newIndex);

  if (cursor != nullptr) {
    if (!isAllocationFailure()) {
      cursor->setLayoutReference(nextPointedLayout);
      cursor->setPosition(nextPosition);
    } else {
      cursor->setLayoutReference(*this);
    }
  }
}

void HorizontalLayoutRef::removeEmptyChildBeforeInsertionAtIndex(int * index, int * currentNumberOfChildren, bool shouldRemoveOnLeft, LayoutCursor * cursor) {
  if (isAllocationFailure()) {
    return;
  }
  int childrenCount = currentNumberOfChildren == nullptr ? numberOfChildren() : *currentNumberOfChildren;
  assert(*index >= 0 && *index <= childrenCount);
  /* If empty, remove the child that would be on the right of the inserted
   * layout. */
  if (*index < childrenCount) {
    LayoutRef c = childAtIndex(*index);
    if (c.isEmpty()) {
      removeChild(c, cursor, true);
      childrenCount--;
      if (currentNumberOfChildren != nullptr) {
        *currentNumberOfChildren = childrenCount;
      }
    }
  }
  /* If empty, remove the child that would be on the left of the inserted
   * layout. */
  if (shouldRemoveOnLeft && *index - 1 >= 0 && *index - 1 < childrenCount) {
    LayoutRef c = childAtIndex(*index - 1);
    if (c.isEmpty()) {
      removeChild(c, cursor, true);
      *index = *index - 1;
      childrenCount--;
      if (currentNumberOfChildren != nullptr) {
        *currentNumberOfChildren = childrenCount;
      }
    }
  }
}

}
