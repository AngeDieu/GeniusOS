#include <poincare/grid_layout.h>
#include <poincare/layout_helper.h>

#include <algorithm>

namespace Poincare {

// LayoutNode
int GridLayoutNode::indexAfterHorizontalCursorMove(
    OMG::HorizontalDirection direction, int currentIndex,
    bool *shouldRedrawLayout) {
  if (currentIndex == k_outsideIndex) {
    return direction.isLeft() ? numberOfChildren() - 1 : 0;
  }
  if ((direction.isLeft() && childIsLeftOfGrid(currentIndex)) ||
      (direction.isRight() && childIsRightOfGrid(currentIndex))) {
    return k_outsideIndex;
  }
  int step = direction.isLeft() ? -1 : 1;
  return currentIndex + step;
}

int GridLayoutNode::indexAfterVerticalCursorMove(
    OMG::VerticalDirection direction, int currentIndex,
    PositionInLayout positionAtCurrentIndex, bool *shouldRedrawLayout) {
  if (currentIndex == k_outsideIndex) {
    return k_cantMoveIndex;
  }
  if (direction.isUp() && currentIndex >= m_numberOfColumns) {
    return currentIndex - m_numberOfColumns;
  }
  if (direction.isDown() &&
      currentIndex < numberOfChildren() - m_numberOfColumns) {
    return currentIndex + m_numberOfColumns;
  }
  return k_cantMoveIndex;
}

LayoutNode::DeletionMethod GridLayoutNode::deletionMethodForCursorLeftOfChild(
    int childIndex) const {
  if (childIndex == k_outsideIndex) {
    return DeletionMethod::MoveLeft;
  }

  assert(isEditing());
  int row = rowAtChildIndex(childIndex);
  int column = columnAtChildIndex(childIndex);
  if (row == 0 && column == 0 &&
      numberOfColumns() == k_minimalNumberOfRowsAndColumnsWhileEditing &&
      numberOfRows() == k_minimalNumberOfRowsAndColumnsWhileEditing) {
    /* If the top left child is filled and the cursor is left of it, delete the
     * grid and keep the child. */
    return DeletionMethod::DeleteParent;
  }

  bool deleteWholeRow = !numberOfRowsIsFixed() &&
                        childIsLeftOfGrid(childIndex) &&
                        !childIsBottomOfGrid(childIndex) && isRowEmpty(row);
  bool deleteWholeColumn =
      !numberOfColumnsIsFixed() && childIsTopOfGrid(childIndex) &&
      !childIsRightOfGrid(childIndex) && isColumnEmpty(column);
  if (deleteWholeRow || deleteWholeColumn) {
    /* Pressing backspace at the top of an empty column or a the left of an
     * empty row deletes the whole column/row. */
    return deleteWholeRow && deleteWholeColumn
               ? DeletionMethod::GridLayoutDeleteColumnAndRow
               : (deleteWholeRow ? DeletionMethod::GridLayoutDeleteRow
                                 : DeletionMethod::GridLayoutDeleteColumn);
  }

  if (childIsLeftOfGrid(childIndex) && row != 0) {
    return DeletionMethod::GridLayoutMoveToUpperRow;
  }
  return DeletionMethod::MoveLeft;
}

void GridLayoutNode::willFillEmptyChildAtIndex(int childIndex) {
  assert(childAtIndex(childIndex)->isEmpty());
  assert(isEditing());
  bool isBottomOfGrid = childIsBottomOfGrid(childIndex);
  bool isRightOfGrid = childIsRightOfGrid(childIndex);
  if (isRightOfGrid && !numberOfColumnsIsFixed()) {
    assert(static_cast<HorizontalLayoutNode *>(childAtIndex(childIndex))
               ->emptyColor() == EmptyRectangle::Color::Gray);
    colorGrayEmptyLayoutsInYellowInColumnOrRow(true, m_numberOfColumns - 1);
    addEmptyColumn(EmptyRectangle::Color::Gray);
  }
  if (isBottomOfGrid && !numberOfRowsIsFixed()) {
    assert(static_cast<HorizontalLayoutNode *>(childAtIndex(childIndex))
                   ->emptyColor() == EmptyRectangle::Color::Gray ||
           isRightOfGrid);  // The empty color already changed if isRightOfGrid
    colorGrayEmptyLayoutsInYellowInColumnOrRow(false, m_numberOfRows - 1);
    addEmptyRow(EmptyRectangle::Color::Gray);
  }
}

int GridLayoutNode::removeTrailingEmptyRowOrColumnAtChildIndex(int childIndex) {
  assert(childAtIndex(childIndex)->isEmpty());
  assert(isEditing());
  int row = rowAtChildIndex(childIndex);
  int column = columnAtChildIndex(childIndex);
  bool isRightOfGrid = childIsInLastNonGrayColumn(childIndex);
  bool isBottomOfGrid = childIsInLastNonGrayRow(childIndex);
  int newColumn = column;
  int newRow = row;
  while (isRightOfGrid && !numberOfColumnsIsFixed() &&
         numberOfColumns() > k_minimalNumberOfRowsAndColumnsWhileEditing &&
         isColumnEmpty(column)) {
    newColumn = column;
    deleteColumnAtIndex(column--);
  }
  while (isBottomOfGrid && !numberOfRowsIsFixed() &&
         numberOfRows() > k_minimalNumberOfRowsAndColumnsWhileEditing &&
         isRowEmpty(row)) {
    newRow = row;
    deleteRowAtIndex(row--);
  }
  assert(numberOfColumns() >= k_minimalNumberOfRowsAndColumnsWhileEditing &&
         numberOfRows() >= k_minimalNumberOfRowsAndColumnsWhileEditing);
  return indexAtRowColumn(newRow, newColumn);
}

// Protected
void GridLayoutNode::deleteRowAtIndex(int index) {
  assert(!numberOfRowsIsFixed());
  assert(index >= 0 && index < m_numberOfRows);
  GridLayout thisRef = GridLayout(this);
  /* removeChildAtIndexInPlace messes with the number of rows to keep it
   * consistent with the number of children */
  int numberOfColumns = m_numberOfColumns;
  int numberOfRows = m_numberOfRows;
  for (int i = 0; i < numberOfColumns; i++) {
    thisRef.removeChildAtIndexInPlace(index * numberOfColumns);
  }
  thisRef.setDimensions(numberOfRows - 1, numberOfColumns);
}

void GridLayoutNode::deleteColumnAtIndex(int index) {
  assert(!numberOfColumnsIsFixed());
  assert(index >= 0 && index < m_numberOfColumns);
  GridLayout thisRef = GridLayout(this);
  /* removeChildAtIndexInPlace messes with the number of rows to keep it
   * consistent with the number of children */
  int numberOfColumns = m_numberOfColumns;
  int numberOfRows = m_numberOfRows;
  for (int i = (numberOfRows - 1) * numberOfColumns + index; i > -1;
       i -= numberOfColumns) {
    thisRef.removeChildAtIndexInPlace(i);
  }
  thisRef.setDimensions(numberOfRows, numberOfColumns - 1);
}

bool GridLayoutNode::childIsLeftOfGrid(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return columnAtChildIndex(index) == 0;
}

bool GridLayoutNode::childIsRightOfGrid(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return columnAtChildIndex(index) == m_numberOfColumns - 1;
}

bool GridLayoutNode::childIsTopOfGrid(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return rowAtChildIndex(index) == 0;
}

bool GridLayoutNode::childIsBottomOfGrid(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return rowAtChildIndex(index) == m_numberOfRows - 1;
}

bool GridLayoutNode::childIsInLastNonGrayColumn(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return columnAtChildIndex(index) == m_numberOfColumns - 1 - isEditing();
}

bool GridLayoutNode::childIsInLastNonGrayRow(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return rowAtChildIndex(index) == m_numberOfRows - 1 - isEditing();
}

int GridLayoutNode::rowAtChildIndex(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return (int)(index / m_numberOfColumns);
}

int GridLayoutNode::columnAtChildIndex(int index) const {
  assert(index >= 0 && index < m_numberOfRows * m_numberOfColumns);
  return index - m_numberOfColumns * rowAtChildIndex(index);
}

int GridLayoutNode::indexAtRowColumn(int row, int column) const {
  assert(row >= 0 && row < m_numberOfRows);
  assert(column >= 0 && column < m_numberOfColumns);
  return row * m_numberOfColumns + column;
}

int GridLayoutNode::closestNonGrayIndex(int index) const {
  if (!isEditing()) {
    return index;
  }
  int row = rowAtChildIndex(index);
  int column = columnAtChildIndex(index);
  if (!numberOfColumnsIsFixed() && childIsRightOfGrid(index)) {
    column--;
  }
  if (!numberOfRowsIsFixed() && childIsBottomOfGrid(index)) {
    row--;
  }
  return indexAtRowColumn(row, column);
}

KDSize GridLayoutNode::computeSize(KDFont::Size font) { return gridSize(font); }

KDCoordinate GridLayoutNode::computeBaseline(KDFont::Size font) {
  return (gridHeight(font) + 1) / 2;
}

KDPoint GridLayoutNode::positionOfChild(LayoutNode *l, KDFont::Size font) {
  int childIndex = indexOfChild(l);
  int row = rowAtChildIndex(childIndex);
  int column = columnAtChildIndex(childIndex);
  KDCoordinate x = 0;
  for (int j = 0; j < column; j++) {
    x += columnWidth(j, font);
  }
  x += (columnWidth(column, font) - l->layoutSize(font).width()) / 2 +
       column * horizontalGridEntryMargin(font);
  KDCoordinate y = 0;
  for (int i = 0; i < row; i++) {
    y += rowHeight(i, font);
  }
  y += rowBaseline(row, font) - l->baseline(font) +
       row * verticalGridEntryMargin(font);
  return KDPoint(x, y);
}

// Private

KDCoordinate GridLayoutNode::rowBaseline(int row, KDFont::Size font) {
  assert(m_numberOfColumns > 0);
  KDCoordinate rowBaseline = 0;
  int column = 0;
  for (LayoutNode *l : childrenFromIndex(row * m_numberOfColumns)) {
    rowBaseline = std::max(rowBaseline, l->baseline(font));
    column++;
    if (column >= m_numberOfColumns) {
      break;
    }
  }
  return rowBaseline;
}

KDCoordinate GridLayoutNode::rowHeight(int row, KDFont::Size font) const {
  KDCoordinate underBaseline = 0;
  KDCoordinate aboveBaseline = 0;
  int column = 0;
  for (LayoutNode *l : const_cast<GridLayoutNode *>(this)->childrenFromIndex(
           row * m_numberOfColumns)) {
    KDCoordinate b = l->baseline(font);
    underBaseline =
        std::max<KDCoordinate>(underBaseline, l->layoutSize(font).height() - b);
    aboveBaseline = std::max(aboveBaseline, b);
    column++;
    if (column >= m_numberOfColumns) {
      break;
    }
  }
  return aboveBaseline + underBaseline;
}

KDCoordinate GridLayoutNode::gridHeight(KDFont::Size font) const {
  KDCoordinate totalHeight = 0;
  for (int row = 0; row < m_numberOfRows; row++) {
    totalHeight += rowHeight(row, font);
  }
  totalHeight += m_numberOfRows > 0
                     ? (m_numberOfRows - 1) * verticalGridEntryMargin(font)
                     : 0;
  return totalHeight;
}

KDCoordinate GridLayoutNode::columnWidth(int column, KDFont::Size font) const {
  KDCoordinate columnWidth = 0;
  int childIndex = column;
  int lastIndex = (m_numberOfRows - 1) * m_numberOfColumns + column;
  for (LayoutNode *l :
       const_cast<GridLayoutNode *>(this)->childrenFromIndex(column)) {
    if (childIndex % m_numberOfColumns == column) {
      columnWidth = std::max(columnWidth, l->layoutSize(font).width());
      if (childIndex >= lastIndex) {
        break;
      }
    }
    childIndex++;
  }
  return columnWidth;
}

KDCoordinate GridLayoutNode::gridWidth(KDFont::Size font) const {
  KDCoordinate totalWidth = 0;
  for (int j = 0; j < m_numberOfColumns; j++) {
    totalWidth += columnWidth(j, font);
  }
  totalWidth += m_numberOfColumns > 0
                    ? (m_numberOfColumns - 1) * horizontalGridEntryMargin(font)
                    : 0;
  return totalWidth;
}

bool GridLayoutNode::isColumnOrRowEmpty(bool column, int index) const {
  assert(index >= 0 && index < (column ? m_numberOfColumns : m_numberOfRows));
  int i = index * (column ? 1 : m_numberOfColumns);
  int startingIndex = i;
  for (LayoutNode *l :
       const_cast<GridLayoutNode *>(this)->childrenFromIndex(startingIndex)) {
    if ((column && i > index + (m_numberOfRows - 1) * m_numberOfColumns) ||
        (!column && i >= (index + 1) * m_numberOfColumns)) {
      break;
    }
    if ((!column || i % m_numberOfColumns == index) && !l->isEmpty()) {
      return false;
    }
    i++;
  }
  return true;
}

void GridLayoutNode::addEmptyRowOrColumn(bool column,
                                         EmptyRectangle::Color color) {
  GridLayout thisRef = GridLayout(this);
  /* addChildAtIndexInPlace messes with the number of rows to keep it consistent
   * with the number of children */
  int previousNumberOfChildren = numberOfChildren();
  int previousNumberOfLines = column ? m_numberOfColumns : m_numberOfRows;
  int otherNumberOfLines = column ? m_numberOfRows : m_numberOfColumns;
  for (int i = 0; i < otherNumberOfLines; i++) {
    HorizontalLayout h = HorizontalLayout::Builder();
    h.setEmptyColor(color);
    thisRef.addChildAtIndexInPlace(
        h,
        column ? (i + 1) * (previousNumberOfLines + 1) - 1
               : previousNumberOfChildren,
        previousNumberOfChildren + i);
    // WARNING: Do not access "this" afterwards
  }
  if (column) {
    thisRef.setDimensions(otherNumberOfLines, previousNumberOfLines + 1);
  } else {
    thisRef.setDimensions(previousNumberOfLines + 1, otherNumberOfLines);
  }
}

void GridLayoutNode::colorGrayEmptyLayoutsInYellowInColumnOrRow(bool column,
                                                                int lineIndex) {
  int childIndex = lineIndex * (column ? 1 : m_numberOfColumns);
  int startIndex = childIndex;
  int maxIndex =
      column ? (m_numberOfRows - 1 - static_cast<int>(!numberOfRowsIsFixed())) *
                       m_numberOfColumns +
                   lineIndex
             : lineIndex * m_numberOfColumns + m_numberOfColumns - 1 -
                   static_cast<int>(!numberOfColumnsIsFixed());
  for (LayoutNode *lastLayoutOfLine : childrenFromIndex(startIndex)) {
    if (childIndex > maxIndex) {
      break;
    }
    if ((!column || childIndex % m_numberOfColumns == lineIndex) &&
        lastLayoutOfLine->isEmpty()) {
      assert(lastLayoutOfLine->isHorizontal());
      static_cast<HorizontalLayoutNode *>(lastLayoutOfLine)
          ->setEmptyColor(EmptyRectangle::Color::Yellow);
    }
    childIndex++;
  }
}

// Grid Layout Reference
void GridLayout::setDimensions(int rows, int columns) {
  assert(rows * columns == numberOfChildren());
  setNumberOfRows(rows);
  setNumberOfColumns(columns);
}

}  // namespace Poincare
