#ifndef CALCULATION_ADDITIONAL_OUTPUTS_ILLUSTRATED_EXPRESSIONS_LIST_CONTROLLER_H
#define CALCULATION_ADDITIONAL_OUTPUTS_ILLUSTRATED_EXPRESSIONS_LIST_CONTROLLER_H

#include <apps/i18n.h>
#include <poincare/variable_context.h>

#include "../calculation_store.h"
#include "chained_expressions_list_controller.h"
#include "illustration_cell.h"

namespace Calculation {

class IllustratedExpressionsListController
    : public ChainedExpressionsListController,
      public Escher::SelectableListViewDelegate {
 public:
  IllustratedExpressionsListController(
      EditExpressionController* editExpressionController,
      bool highlightWholeCells = false)
      : ChainedExpressionsListController(editExpressionController,
                                         highlightWholeCells, this) {}

  // Responder
  void didBecomeFirstResponder() override;

  // MemoizedListViewDataSource
  int numberOfRows() const override;
  int reusableCellCount(int type) override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  KDCoordinate nonMemoizedRowHeight(int row) override;
  int typeAtRow(int row) const override {
    return row == 0 && showIllustration() ? k_illustrationCellType
                                          : k_expressionCellType;
  }
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  bool cellAtLocationIsSelectable(int column, int row) override {
    return typeAtRow(row) != k_illustrationCellType;
  }

  // SelectableListViewDelegate
  void listViewDidChangeSelectionAndDidScroll(
      Escher::SelectableListView* t, int previousSelectedRow,
      KDPoint previousOffset, bool withinTemporarySelection) override;

  /* Since we can't focus the illustration, we need to keep its height small
   * enough for a sufficient part of the first expressionCell to be visible when
   * the focus is there and the table view scrolled at the top. */
  constexpr static KDCoordinate k_illustrationHeight = 120;

 protected:
  constexpr static uint8_t k_illustrationCellType = 0;
  constexpr static uint8_t k_expressionCellType = 1;
  bool showIllustration() const { return m_showIllustration; }
  void setShowIllustration(bool showIllustration) {
    m_showIllustration = showIllustration;
  }
  /* Helper to add a line in the format func(arg) = exact ~ approx */
  void setLineAtIndex(int index, Poincare::Expression formula,
                      Poincare::Expression expression,
                      Poincare::Context* context,
                      Poincare::Preferences* preferences);

 private:
  int textAtIndex(char* buffer, size_t bufferSize, Escher::HighlightCell* cell,
                  int index) override;
  // Cells
  virtual IllustrationCell* illustrationCell() = 0;
  bool m_showIllustration;
};

}  // namespace Calculation

#endif
