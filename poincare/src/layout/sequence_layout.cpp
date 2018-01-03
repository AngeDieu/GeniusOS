#include "sequence_layout.h"
#include "parenthesis_left_layout.h"
#include "parenthesis_right_layout.h"
#include "uneditable_horizontal_trio_layout.h"
#include <poincare/expression_layout_cursor.h>
#include <poincare/expression_layout_array.h>
#include <string.h>
#include <assert.h>

namespace Poincare {

SequenceLayout::SequenceLayout(ExpressionLayout * argument, ExpressionLayout * lowerBound, ExpressionLayout * upperBound, bool cloneOperands) :
  StaticLayoutHierarchy()
{
  ParenthesisLeftLayout * parLeft = new ParenthesisLeftLayout();
  ParenthesisRightLayout * parRight = new ParenthesisRightLayout();
  UneditableHorizontalTrioLayout * horLayout = new UneditableHorizontalTrioLayout(parLeft, argument, parRight, cloneOperands, false);
  build(ExpressionLayoutArray(horLayout, lowerBound, upperBound).array(), 3, cloneOperands);
  if (cloneOperands) {
    delete parLeft;
    delete parRight;
  }
}

void SequenceLayout::backspaceAtCursor(ExpressionLayoutCursor * cursor) {
  // Case: Left of the bounds or of the argument.
  // Delete the sequence, keep the argument.
  if (cursor->position() == ExpressionLayoutCursor::Position::Left
      && ((lowerBoundLayout()
          && cursor->pointedExpressionLayout() == lowerBoundLayout())
        || (upperBoundLayout()
          && cursor->pointedExpressionLayout() == upperBoundLayout())
        || cursor->pointedExpressionLayout() == argumentLayout()))
  {
    ExpressionLayout * previousParent = m_parent;
    int indexInParent = previousParent->indexOfChild(this);
    replaceWith(argumentLayout(), true);
    // Place the cursor on the right of the left brother of the sequence if
    // there is one.
    if (indexInParent > 0) {
      cursor->setPointedExpressionLayout(previousParent->editableChild(indexInParent - 1));
      cursor->setPosition(ExpressionLayoutCursor::Position::Right);
      return;
    }
    // Else place the cursor on the Left of the parent.
    cursor->setPointedExpressionLayout(previousParent);
    return;
  }
  // Case: Right.
  // Move inside the argument.
  if (cursor->pointedExpressionLayout() == this
      && cursor->position() == ExpressionLayoutCursor::Position::Right)
  {
    cursor->setPointedExpressionLayout(argumentLayout());
    return;
  }
  // Case: Left.
  // Ask the parent.
  assert(cursor->position() == ExpressionLayoutCursor::Position::Left);
  m_parent->backspaceAtCursor(cursor);
}

bool SequenceLayout::moveLeft(ExpressionLayoutCursor * cursor) {
  // Case: Left of the bounds.
  // Go Left of the sequence.
  if (cursor->position() == ExpressionLayoutCursor::Position::Left
      && ((lowerBoundLayout()
          && cursor->pointedExpressionLayout() == lowerBoundLayout())
        || (upperBoundLayout()
          && cursor->pointedExpressionLayout() == upperBoundLayout())))
  {
    cursor->setPointedExpressionLayout(this);
    return true;
  }
  // Case: Left of the argument.
  // Go Right of the lower bound.
  if (cursor->position() == ExpressionLayoutCursor::Position::Left
      && argumentLayout()
      && cursor->pointedExpressionLayout() == argumentWithParenthesesLayout())
  {
    assert(lowerBoundLayout() != nullptr);
    cursor->setPointedExpressionLayout(lowerBoundLayout()->editableChild(1));
    cursor->setPosition(ExpressionLayoutCursor::Position::Right);
    return true;
  }
  assert(cursor->pointedExpressionLayout() == this);
  // Case: Right.
  // Go to the argument and move Left.
  if (cursor->position() == ExpressionLayoutCursor::Position::Right) {
    assert(argumentLayout() != nullptr);
    cursor->setPointedExpressionLayout(argumentLayout());
    return true;
  }
  assert(cursor->position() == ExpressionLayoutCursor::Position::Left);
  // Case: Left.
  // Ask the parent.
  if (m_parent) {
    return m_parent->moveLeft(cursor);
  }
  return false;
}

bool SequenceLayout::moveRight(ExpressionLayoutCursor * cursor) {
  // Case: Right of the bounds.
  // Go Left of the argument.
  if (cursor->position() == ExpressionLayoutCursor::Position::Right
      && ((lowerBoundLayout()
          && cursor->pointedExpressionLayout() == lowerBoundLayout())
        || (upperBoundLayout()
          && cursor->pointedExpressionLayout() == upperBoundLayout())))
  {
    assert(argumentLayout() != nullptr);
    cursor->setPointedExpressionLayout(argumentLayout());
    cursor->setPosition(ExpressionLayoutCursor::Position::Left);
    return true;
  }
  // Case: Right of the argument.
  // Ask the parent.
  if (cursor->position() == ExpressionLayoutCursor::Position::Right
      && argumentLayout()
      && cursor->pointedExpressionLayout() == argumentWithParenthesesLayout())
  {
    cursor->setPointedExpressionLayout(this);
    cursor->setPosition(ExpressionLayoutCursor::Position::Right);
    return true;
  }
  assert(cursor->pointedExpressionLayout() == this);
  // Case: Left.
  // Go to the upper bound.
  if (cursor->position() == ExpressionLayoutCursor::Position::Left) {
    assert(upperBoundLayout() != nullptr);
    cursor->setPointedExpressionLayout(upperBoundLayout());
    return true;
  }
  assert(cursor->position() == ExpressionLayoutCursor::Position::Right);
  // Case: Right.
  // Ask the parent.
  if (m_parent) {
    return m_parent->moveRight(cursor);
  }
  return false;
}

bool SequenceLayout::moveUp(ExpressionLayoutCursor * cursor, ExpressionLayout * previousLayout, ExpressionLayout * previousPreviousLayout) {
  // If the cursor is inside the lower bound, move it to the upper bound.
  if (lowerBoundLayout() && previousLayout == lowerBoundLayout()) {
    assert(upperBoundLayout() != nullptr);
    return upperBoundLayout()->moveUpInside(cursor);
  }
  // If the cursor is Left of the argument, move it to the upper bound.
  if (argumentLayout()
      && cursor->positionIsEquivalentTo(argumentLayout(), ExpressionLayoutCursor::Position::Left))
  {
    assert(upperBoundLayout() != nullptr);
    return upperBoundLayout()->moveUpInside(cursor);
  }
  return ExpressionLayout::moveUp(cursor, previousLayout, previousPreviousLayout);
}
bool SequenceLayout::moveDown(ExpressionLayoutCursor * cursor, ExpressionLayout * previousLayout, ExpressionLayout * previousPreviousLayout) {
  // If the cursor is inside the upper bound, move it to the lower bound.
  if (upperBoundLayout() && previousLayout == upperBoundLayout()) {
    assert(lowerBoundLayout() != nullptr);
    return lowerBoundLayout()->editableChild(1)->moveDownInside(cursor);
  }
  // If the cursor is Left of the argument, move it to the lower bound.
  if (argumentLayout()
      && cursor->positionIsEquivalentTo(argumentLayout(), ExpressionLayoutCursor::Position::Left))
  {
    assert(lowerBoundLayout() != nullptr);
    return lowerBoundLayout()->moveDownInside(cursor);
  }
  return ExpressionLayout::moveDown(cursor, previousLayout, previousPreviousLayout);
}

char SequenceLayout::XNTChar() const {
  return 'n';
}

int SequenceLayout::writeDerivedClassInBuffer(const char * operatorName, char * buffer, int bufferSize) const {
  assert(operatorName != nullptr);
  if (bufferSize == 0) {
    return -1;
  }
  buffer[bufferSize-1] = 0;

  // Write the operator name
  int numberOfChar = strlcpy(buffer, operatorName, bufferSize);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the opening parenthesis
  buffer[numberOfChar++] = '(';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the argument
  numberOfChar += const_cast<SequenceLayout *>(this)->argumentLayout()->writeTextInBuffer(buffer+numberOfChar, bufferSize-numberOfChar);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the comma
  buffer[numberOfChar++] = ',';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the lower bound without the "n="
  numberOfChar += LayoutEngine::writeInfixExpressionLayoutTextInBuffer(const_cast<SequenceLayout *>(this)->lowerBoundLayout(), buffer+numberOfChar, bufferSize-numberOfChar, "", 1);
  // TODO This works because the lower bound layout should always be an
  // horizontal layout.
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the comma
  buffer[numberOfChar++] = ',';
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the upper bound
  numberOfChar += const_cast<SequenceLayout *>(this)->upperBoundLayout()->writeTextInBuffer(buffer+numberOfChar, bufferSize-numberOfChar);
  if (numberOfChar >= bufferSize-1) { return bufferSize-1; }

  // Write the closing parenthesis
  buffer[numberOfChar++] = ')';
  buffer[numberOfChar] = 0;
  return numberOfChar;
}

ExpressionLayout * SequenceLayout::upperBoundLayout() {
  return editableChild(2);
}

ExpressionLayout * SequenceLayout::lowerBoundLayout() {
  return editableChild(1);
}

ExpressionLayout * SequenceLayout::argumentLayout() {
  return argumentWithParenthesesLayout()->editableChild(1);
}

ExpressionLayout * SequenceLayout::argumentWithParenthesesLayout() {
  return editableChild(0);
}

KDSize SequenceLayout::computeSize() {
  KDSize argumentSize = argumentWithParenthesesLayout()->size();
  KDSize lowerBoundSize = lowerBoundLayout()->size();
  KDSize upperBoundSize = upperBoundLayout()->size();
  return KDSize(
    max(max(k_symbolWidth, lowerBoundSize.width()), upperBoundSize.width())+k_argumentWidthMargin+argumentSize.width(),
    baseline() + max(k_symbolHeight/2+k_boundHeightMargin+lowerBoundSize.height(), argumentSize.height() - argumentLayout()->baseline())
  );
}

void SequenceLayout::computeBaseline() {
  m_baseline = max(upperBoundLayout()->size().height()+k_boundHeightMargin+(k_symbolHeight+1)/2, argumentLayout()->baseline());
  m_baselined = true;
}

KDPoint SequenceLayout::positionOfChild(ExpressionLayout * child) {
  KDSize lowerBoundSize = lowerBoundLayout()->size();
  KDSize upperBoundSize = upperBoundLayout()->size();
  KDCoordinate x = 0;
  KDCoordinate y = 0;
  if (child == lowerBoundLayout()) {
    x = max(max(0, (k_symbolWidth-lowerBoundSize.width())/2), (upperBoundSize.width()-lowerBoundSize.width())/2);
    y = baseline() + k_symbolHeight/2 + k_boundHeightMargin;
  } else if (child == upperBoundLayout()) {
    x = max(max(0, (k_symbolWidth-upperBoundSize.width())/2), (lowerBoundSize.width()-upperBoundSize.width())/2);
    y = baseline() - (k_symbolHeight+1)/2- k_boundHeightMargin-upperBoundSize.height();
  } else if (child == argumentWithParenthesesLayout()) {
    x = max(max(k_symbolWidth, lowerBoundSize.width()), upperBoundSize.width())+k_argumentWidthMargin;
    y = baseline() - argumentLayout()->baseline();
  } else {
    assert(false);
  }
  return KDPoint(x,y);
}

}
