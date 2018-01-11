#ifndef POINCARE_EXPRESSION_LAYOUT_H
#define POINCARE_EXPRESSION_LAYOUT_H

#include <kandinsky.h>

namespace Poincare {

class ExpressionLayoutCursor;

class ExpressionLayout {
public:
  enum class VerticalDirection {
    Up,
    Down
  };
  enum class HorizontalDirection {
    Left,
    Right
  };

  /* Constructor & Destructor */
  ExpressionLayout();
  virtual  ~ExpressionLayout() = default;
  virtual ExpressionLayout * clone() const = 0;

  /* Rendering */
  void draw(KDContext * ctx, KDPoint p, KDColor expressionColor = KDColorBlack, KDColor backgroundColor = KDColorWhite);
  KDPoint origin();
  KDPoint absoluteOrigin();
  KDSize size();
  KDCoordinate baseline();
  virtual void invalidAllSizesPositionsAndBaselines();

  /* Hierarchy */

  // Children
  virtual const ExpressionLayout * const * children() const = 0;
  const ExpressionLayout * child(int i) const;
  ExpressionLayout * editableChild(int i) { return const_cast<ExpressionLayout *>(child(i)); }
  virtual int numberOfChildren() const = 0;
  int indexOfChild(const ExpressionLayout * child) const;

  // Parent
  void setParent(ExpressionLayout * parent) { m_parent = parent; }
  const ExpressionLayout * parent() const { return m_parent; }
  ExpressionLayout * editableParent() { return m_parent; }
  bool hasAncestor(const ExpressionLayout * e) const;

  /* Dynamic Layout */

  // Add
  virtual bool addChildAtIndex(ExpressionLayout * child, int index) { return false; }
  virtual void addBrother(ExpressionLayoutCursor * cursor, ExpressionLayout * brother);

  // Replace
  virtual ExpressionLayout * replaceWith(ExpressionLayout * newChild, bool deleteAfterReplace = true);
  ExpressionLayout * replaceWithAndMoveCursor(ExpressionLayout * newChild, bool deleteAfterReplace, ExpressionLayoutCursor * cursor);
  virtual void replaceChild(const ExpressionLayout * oldChild, ExpressionLayout * newChild, bool deleteOldChild = true);
  virtual void replaceChildAndMoveCursor(
      const ExpressionLayout * oldChild,
      ExpressionLayout * newChild,
      bool deleteOldChild,
      ExpressionLayoutCursor * cursor);

  // Detach
  void detachChild(const ExpressionLayout * e); // Detach a child WITHOUT deleting it
  void detachChildren(); // Detach all children WITHOUT deleting them

  // Remove
  virtual void removeChildAtIndex(int index, bool deleteAfterRemoval);
  virtual void removePointedChildAtIndexAndMoveCursor(int index, bool deleteAfterRemoval, ExpressionLayoutCursor * cursor);

  // User input
  bool insertLayoutAtCursor(ExpressionLayout * newChild, ExpressionLayoutCursor * cursor);
  virtual void backspaceAtCursor(ExpressionLayoutCursor * cursor);

  /* Tree navigation */
  virtual bool moveLeft(ExpressionLayoutCursor * cursor) = 0;
  virtual bool moveRight(ExpressionLayoutCursor * cursor) = 0;
  virtual bool moveUp(
      ExpressionLayoutCursor * cursor,
      ExpressionLayout * previousLayout = nullptr,
      ExpressionLayout * previousPreviousLayout = nullptr);
  virtual bool moveDown(
      ExpressionLayoutCursor * cursor,
      ExpressionLayout * previousLayout = nullptr,
      ExpressionLayout * previousPreviousLayout = nullptr);
  virtual bool moveUpInside(ExpressionLayoutCursor * cursor);
  virtual bool moveDownInside(ExpressionLayoutCursor * cursor);

  /* Expression Engine */
  virtual int writeTextInBuffer(char * buffer, int bufferSize) const = 0;

  /* Other */
  virtual bool isCollapsable(int * numberOfOpenParenthesis, bool goingLeft) const { return true; }
  /* isCollapsable is used when adding a brother fraction: should the layout be
   * inserted in the numerator (or denominator)? */
  virtual bool mustHaveLeftBrother() const { return false; }
  virtual bool isHorizontal() const { return false; }
  virtual bool isLeftParenthesis() const { return false; }
  virtual bool isRightParenthesis() const { return false; }
  virtual bool isLeftBracket() const { return false; }
  virtual bool isRightBracket() const { return false; }
  virtual bool isEmpty() const { return false; }
  virtual bool isMatrix() const { return false; }
  virtual char XNTChar() const;

protected:
  virtual void render(KDContext * ctx, KDPoint p, KDColor expressionColor, KDColor backgroundColor) = 0;
  virtual KDSize computeSize() = 0;
  virtual void computeBaseline() = 0;
  virtual KDPoint positionOfChild(ExpressionLayout * child) = 0;
  ExpressionLayout * m_parent;
  KDCoordinate m_baseline;
  /* m_baseline is the signed vertical distance from the top of the layout to
   * the fraction bar of an hypothetical fraction brother layout. If the top of
   * the layout is under that bar, the baseline is negative. */
  bool m_sized;
  bool m_baselined;
  bool m_positioned;
private:
  void detachChildAtIndex(int i);
  bool moveInside(VerticalDirection direction, ExpressionLayoutCursor * cursor);
  void moveCursorInsideAtDirection (
    VerticalDirection direction,
    ExpressionLayoutCursor * cursor,
    ExpressionLayout ** childResult,
    void * resultPosition,
    int * resultScore);
  ExpressionLayout * replaceWithJuxtapositionOf(ExpressionLayout * leftChild, ExpressionLayout * rightChild, bool deleteAfterReplace);
  KDRect m_frame;
};

}

#endif
