#include <escher/metric.h>
#include <escher/table_view.h>

extern "C" {
#include <assert.h>
}
#include <algorithm>

namespace Escher {

TableView::TableView(TableViewDataSource* dataSource,
                     ScrollViewDataSource* scrollDataSource)
    : ScrollView(&m_contentView, scrollDataSource),
      m_contentView(this, dataSource, 0, Metric::CellSeparatorThickness) {
  m_decorator.setVisibility(true);
}

void TableView::initSize(KDRect rect) {
  if (bounds().width() <= 0 || bounds().height() <= 0) {
    setSize(KDSize(rect.width(), rect.height()));
    dataSource()->initCellSize(this);
  }
}

void TableView::reloadVisibleCellsAtColumn(int column) {
  // Reload visible cells of the selected column
  int firstVisibleCol = firstDisplayedColumnIndex();
  int lastVisibleCol = firstVisibleCol + numberOfDisplayableColumns();
  if (column < firstVisibleCol || column >= lastVisibleCol) {
    // Column is not visible
    return;
  }
  int firstVisibleRow = firstDisplayedRowIndex();
  int lastVisibleRow = std::max(firstVisibleRow + numberOfDisplayableRows(),
                                totalNumberOfRows() - 1);

  for (int row = firstVisibleRow; row <= lastVisibleRow; row++) {
    reloadCellAtLocation(column, row);
  }
}

void TableView::layoutSubviews(bool force) {
  /* We need to relayout m_contentView to reload data and sizes but also
   * ScrollView if the offset or m_contentView's size changed.
   * On the one hand, ScrollView::layoutSubviews() set m_contentView's frame,
   * which calls layoutSubviews() over m_contentView IF the frame is changed.
   * On the other hand, m_contentView.layoutSubviews() does not relayout
   * ScrollView. For those reasons, we call both of them explicitly. Besides, we
   * must call relayout m_contentView first in order to reload the table's data,
   * otherwise the table's size might be miscomputed for ScrollView.
   * FIXME: This solution is not optimal since layoutSubviews can be called
   * twice over m_contentView. */
  dataSource()->initCellSize(this);
  m_contentView.layoutSubviews(force, true);
  ScrollView::layoutSubviews(force);
}

/* TableView::ContentView */

TableView::ContentView::ContentView(TableView* tableView,
                                    TableViewDataSource* dataSource,
                                    KDCoordinate horizontalCellOverlap,
                                    KDCoordinate verticalCellOverlap)
    : View(),
      m_tableView(tableView),
      m_dataSource(dataSource),
      m_horizontalCellOverlap(horizontalCellOverlap),
      m_verticalCellOverlap(verticalCellOverlap) {}

void TableView::ContentView::drawRect(KDContext* ctx, KDRect rect) const {
  /* The separators between cells need to be filled with background color.
   * Cells frames are already set, so we just need to browse through the
   * cells and detect when there is a gap in their frames coordinates.
   *
   * Since the separators are for whole columns and rows, it is not necessary
   * to browse through all the cells, so we just go through the first row and
   * the first column.
   *  */
  KDColor bColor = m_tableView->backgroundColor();
  int nC = numberOfDisplayableColumns();
  int nR = numberOfDisplayableRows();
  for (int k = 0; k < 2; k++) {
    /* If k == 0, draw column separators (go through the first row).
     * If k == 1, draw row separators (go through the first column). */
    bool col = k == 0;
    int iMax = col ? nC : nR;
    /* If col, check the gap of abscissa in the subview frames.
     * Else, check the gap of ordinates.
     * "c" designate the current coordinate (abscissa or ordinate). */
    KDCoordinate c = 0;
    // Lenght of the filled rectangle.
    KDCoordinate length = col ? bounds().height() : bounds().width();
    for (int i = 0; i < iMax; i++) {
      const View* sv =
          const_cast<TableView::ContentView*>(this)->cellAtRelativeLocation(
              col ? i : 0, col ? 0 : i);
      KDRect subviewFrame = relativeChildFrame(sv);
      if (subviewFrame == KDRectZero) {
        continue;
      }
      KDCoordinate cCell = col ? subviewFrame.left() : subviewFrame.top();
      if (cCell > c) {
        /* There is a gap between the cell and the current coordinate: draw
         * the separator rectangle.*/
        KDRect r = KDRect(c, 0, cCell - c, length);
        if (!col) {
          r = r.transposed();
        }
        ctx->fillRect(r, bColor);
      }
      c = col ? subviewFrame.right() + 1 : subviewFrame.bottom() + 1;
    }
    // Draw the separator on the right/bottom of the table
    KDCoordinate cMax = col ? bounds().right() + 1 : bounds().bottom() + 1;
    if (c < cMax) {
      KDRect r = KDRect(c, 0, cMax - c, length);
      if (!col) {
        r = r.transposed();
      }
      ctx->fillRect(r, bColor);
    }
  }
}

KDRect TableView::ContentView::cellFrame(int col, int row) const {
  KDCoordinate columnWidth = m_dataSource->columnWidth(col, false);
  KDCoordinate rowHeight = m_dataSource->rowHeight(row, false);
  if (columnWidth == 0 || rowHeight == 0) {
    return KDRectZero;
  }
  if (columnWidth == KDCOORDINATE_MAX) {  // For ListViewDataSource
    columnWidth = m_tableView->maxContentWidthDisplayableWithoutScrolling();
  }
  return KDRect(m_dataSource->cumulatedWidthBeforeIndex(col) +
                    m_dataSource->separatorBeforeColumn(col),
                m_dataSource->cumulatedHeightBeforeIndex(row) +
                    m_dataSource->separatorBeforeRow(row),
                columnWidth + m_horizontalCellOverlap,
                rowHeight + m_verticalCellOverlap);
}

KDCoordinate TableView::ContentView::width() const {
  int result =
      m_dataSource->cumulatedWidthBeforeIndex(m_dataSource->numberOfColumns()) +
      m_horizontalCellOverlap;
  // handle the case of list: cumulatedWidthBeforeIndex() = KDCOORDINATE_MAX
  return result == KDCOORDINATE_MAX
             ? m_tableView->maxContentWidthDisplayableWithoutScrolling()
             : result;
}

void TableView::ContentView::reloadCellAtLocation(int col, int row,
                                                  bool forceSetFrame) {
  HighlightCell* cell = cellAtLocation(col, row);
  if (cell) {
    m_dataSource->willDisplayCellAtLocation(cell, col, row);
    if (forceSetFrame) {
      setChildFrame(cell, cellFrame(col, row), true);
    }
  }
}

int TableView::ContentView::typeOfSubviewAtIndex(int index) const {
  assert(index >= 0);
  int col = absoluteColumnIndexFromSubviewIndex(index);
  int row = absoluteRowIndexFromSubviewIndex(index);
  int type = m_dataSource->typeAtLocation(col, row);
  return type;
}

int TableView::ContentView::typeIndexFromSubviewIndex(int index,
                                                      int type) const {
  int typeIndex = 0;
  for (int k = 0; k < index; k++) {
    if (typeOfSubviewAtIndex(k) == type) {
      typeIndex++;
    }
  }
  assert(typeIndex < m_dataSource->reusableCellCount(type));
  return typeIndex;
}

HighlightCell* TableView::ContentView::cellAtLocation(int col, int row) {
  int relativeColumn = col - columnsScrollingOffset();
  int relativeRow = row - rowsScrollingOffset();
  if (relativeRow < 0 || relativeRow >= numberOfDisplayableRows() ||
      relativeColumn < 0 || relativeColumn >= numberOfDisplayableColumns()) {
    return nullptr;
  }
  int type = m_dataSource->typeAtLocation(col, row);
  int index = relativeRow * numberOfDisplayableColumns() + relativeColumn;
  int typeIndex = typeIndexFromSubviewIndex(index, type);
  return m_dataSource->reusableCell(typeIndex, type);
}

View* TableView::ContentView::subviewAtIndex(int index) {
  /* This is a hack: the redrawing routine tracks a rectangle which has to be
   * redrawn. Thereby, the union of the rectangles that need to be redrawn
   * sometimes covers areas that are uselessly redrawn. We reverse the order of
   * subviews when redrawing the TableView to make it more likely to uselessly
   * redraw the top left cells rather than the bottom right cells. Due to the
   * screen driver specifications, blinking is less visible at the top left
   * corner than at the bottom right. */
  return static_cast<View*>(
      reusableCellAtIndex(numberOfSubviews() - 1 - index));
}

HighlightCell* TableView::ContentView::reusableCellAtIndex(int index) {
  int type = typeOfSubviewAtIndex(index);
  int typeIndex = typeIndexFromSubviewIndex(index, type);
  return m_dataSource->reusableCell(typeIndex, type);
}

void TableView::ContentView::layoutSubviews(bool force,
                                            bool updateCellContent) {
  /* The number of cells might change during the layouting so it needs to be
   * recomputed at each step of the for loop. */
  for (int index = 0; index < numberOfDisplayableCells(); index++) {
    HighlightCell* cell = reusableCellAtIndex(index);
    int col = absoluteColumnIndexFromSubviewIndex(index);
    int row = absoluteRowIndexFromSubviewIndex(index);
    assert(cellAtLocation(col, row) == cell);
    if (updateCellContent) {
      m_dataSource->willDisplayCellAtLocation(cell, col, row);
    }
    /* Cell's content might change and fit in the same frame. LayoutSubviews
     * must be called on each cells even with an unchanged frame. */
    setChildFrame(cell, cellFrame(col, row), true);
  }
}

int TableView::ContentView::numberOfDisplayableRows() const {
  int rowOffset = rowsScrollingOffset();
  int cumulatedHeightOfLastVisiblePixel =
      m_tableView->bounds().height() + invisibleHeight() - 1;
  int cumulatedRowIndexOfLastVisiblePixel =
      m_dataSource->indexAfterCumulatedHeight(
          cumulatedHeightOfLastVisiblePixel);
  return std::min(m_dataSource->numberOfRows(),
                  cumulatedRowIndexOfLastVisiblePixel + 1) -
         rowOffset;
}

int TableView::ContentView::numberOfDisplayableColumns() const {
  int columnOffset = columnsScrollingOffset();
  int cumulatedWidthOfLastVisiblePixel =
      m_tableView->bounds().width() + invisibleWidth() - 1;
  int cumulatedColumnIndexOfLastVisiblePixel =
      m_dataSource->columnAfterCumulatedWidth(cumulatedWidthOfLastVisiblePixel);
  return std::min(m_dataSource->numberOfColumns(),
                  cumulatedColumnIndexOfLastVisiblePixel + 1) -
         columnOffset;
}

}  // namespace Escher
