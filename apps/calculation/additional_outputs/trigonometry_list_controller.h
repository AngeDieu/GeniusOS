#ifndef CALCULATION_ADDITIONAL_OUTPUTS_TRIGONOMETRY_LIST_CONTROLLER_H
#define CALCULATION_ADDITIONAL_OUTPUTS_TRIGONOMETRY_LIST_CONTROLLER_H

#include "illustrated_expressions_list_controller.h"
#include "trigonometry_graph_cell.h"
#include "trigonometry_model.h"

namespace Calculation {

class TrigonometryListController : public IllustratedExpressionsListController {
 public:
  TrigonometryListController(EditExpressionController* editExpressionController)
      : IllustratedExpressionsListController(editExpressionController),
        m_graphCell(&m_model) {}
  void computeAdditionalResults(
      Poincare::Expression inputExpression,
      Poincare::Expression exactExpression,
      Poincare::Expression approximateExpression) override;
  KDCoordinate nonMemoizedRowHeight(int row) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;

  void setTrigonometryType(bool directTrigonometry) {
    m_directTrigonometry = directTrigonometry;
  }

  /* See comment in parent class for why we need this. Since we need to display
   * fractions of radians in the angle cell, we reduce the graph further. */
  constexpr static KDCoordinate k_illustrationHeight = 105;

 private:
  constexpr static CodePoint k_symbol = UCodePointGreekSmallLetterTheta;
  constexpr static CodePoint k_numberOfExpressionRows = 4;
  I18n::Message messageAtIndex(int index) override;
  IllustrationCell* illustrationCell() override { return &m_graphCell; }
  void updateIsStrictlyEqualAtIndex(int index, Poincare::Context* context);
  TrigonometryGraphCell m_graphCell;
  TrigonometryModel m_model;
  bool m_isStrictlyEqual[k_numberOfExpressionRows];
  bool m_directTrigonometry;
};

}  // namespace Calculation

#endif
