#ifndef INFERENCE_STATISTIC_COMPARISON_OPERATOR_POPUP_DATA_SOURCE_H
#define INFERENCE_STATISTIC_COMPARISON_OPERATOR_POPUP_DATA_SOURCE_H

#include <escher/buffer_text_highlight_cell.h>
#include <escher/list_view_data_source.h>

#include "inference/constants.h"
#include "inference/models/statistic/test.h"

namespace Inference {

class ComparisonOperatorPopupDataSource : public Escher::ListViewDataSource {
 public:
  static Poincare::ComparisonNode::OperatorType OperatorTypeForRow(int row);

  ComparisonOperatorPopupDataSource(Test* test) : m_test(test) {}
  int numberOfRows() const override { return k_numberOfOperators; }
  int reusableCellCount(int type) override { return k_numberOfOperators; }
  Escher::SmallBufferTextHighlightCell* reusableCell(int i, int type) override {
    return &m_cells[i];
  }
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;

 private:
  constexpr static int k_cellBufferSize =
      7 /* μ1-μ2 */ + 3 /* op ≠ */ +
      Constants::k_shortFloatNumberOfChars /* float */ + 1 /* \0 */;
  constexpr static int k_numberOfOperators = 3;
  KDCoordinate nonMemoizedRowHeight(int row) override {
    assert(false); /* Not needed because DropdownPopupController takes care of
                      it */
    return 0;
  }
  Escher::SmallBufferTextHighlightCell m_cells[k_numberOfOperators];
  Test* m_test;
};

}  // namespace Inference

#endif
