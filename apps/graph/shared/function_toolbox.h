#ifndef GRAPH_FUNCTION_TOOLBOX_H
#define GRAPH_FUNCTION_TOOLBOX_H

#include <escher/layout_view.h>
#include <escher/menu_cell.h>
#include <poincare/layout.h>

#include "../../math_toolbox.h"

namespace Graph {

class FunctionToolbox : public MathToolbox {
 public:
  enum class AddedCellsContent : uint8_t {
    None,
    ComparisonOperators,
    NegativeInfinity,
    PositiveInfinity,
  };

  FunctionToolbox();

  void setAddedCellsContent(AddedCellsContent content);

  bool handleEvent(Ion::Events::Event event) override;
  int numberOfRows() const override;
  KDCoordinate nonMemoizedRowHeight(int row) override;
  int reusableCellCount(int type) override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  int typeAtRow(int row) const override;

 protected:
  const Escher::ToolboxMessageTree* messageTreeModelAtIndex(
      int index) const override {
    assert(index >= addedCellsAtRoot());
    return MathToolbox::messageTreeModelAtIndex(index - addedCellsAtRoot());
  }
  int controlChecksum() const override;

 private:
  constexpr static int k_addedCellType = 2;
  constexpr static int k_maxNumberOfAddedCells = 2;

  // At root depth, there are additional rows to display.
  int addedCellsAtRoot() const {
    return m_messageTreeModel == rootModel() ? numberOfAddedCells() : 0;
  }
  bool selectAddedCell(int selectedRow);
  int numberOfAddedCells() const;

  Escher::MenuCell<Escher::LayoutView> m_addedCells[k_maxNumberOfAddedCells];
  Poincare::Layout m_addedCellLayout[k_maxNumberOfAddedCells];
  AddedCellsContent m_addedCellsContent;
};

}  // namespace Graph

#endif
