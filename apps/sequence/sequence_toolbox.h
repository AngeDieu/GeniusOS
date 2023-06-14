#ifndef SEQUENCE_SEQUENCE_TOOLBOX_H
#define SEQUENCE_SEQUENCE_TOOLBOX_H

#include <apps/math_toolbox.h>
#include <escher/layout_view.h>
#include <escher/menu_cell.h>
#include <poincare/layout.h>

namespace Sequence {

class SequenceToolbox : public MathToolbox {
 public:
  SequenceToolbox();
  bool handleEvent(Ion::Events::Event event) override;
  int numberOfRows() const override;
  KDCoordinate nonMemoizedRowHeight(int row) override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  int typeAtRow(int row) const override;
  void buildExtraCellsLayouts(const char* sequenceName, int recurrenceDepth);
  void resetExtraCells() { buildExtraCellsLayouts(nullptr, -1); }

 protected:
  const Escher::ToolboxMessageTree* messageTreeModelAtIndex(
      int index) const override {
    assert(index >= addedCellsAtRoot() &&
           index - addedCellsAtRoot() < m_messageTreeModel->numberOfChildren());
    return MathToolbox::messageTreeModelAtIndex(index - addedCellsAtRoot());
  }
  int controlChecksum() const override;

 private:
  constexpr static int k_addedCellType = 2;
  // At root depth, there are additional rows to display.
  int addedCellsAtRoot() const {
    return m_messageTreeModel == rootModel() ? m_numberOfAddedCells : 0;
  }
  bool selectAddedCell(int selectedRow);
  Escher::MenuCell<Escher::LayoutView> m_addedCells[k_maxNumberOfDisplayedRows];
  Poincare::Layout m_addedCellLayout[k_maxNumberOfDisplayedRows];
  int m_numberOfAddedCells;
};

}  // namespace Sequence

#endif
