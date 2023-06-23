#ifndef ESCHER_EDITABLE_EXPRESSION_CELL_H
#define ESCHER_EDITABLE_EXPRESSION_CELL_H

#include <escher/bordered.h>
#include <escher/highlight_cell.h>
#include <escher/layout_field.h>
#include <escher/layout_field_delegate.h>
#include <escher/metric.h>
#include <escher/responder.h>
#include <escher/text_field_delegate.h>
#include <poincare/print_float.h>

namespace Escher {

class EditableExpressionCell : public Bordered,
                               public HighlightCell,
                               public Responder {
 public:
  EditableExpressionCell(Responder* parentResponder = nullptr,
                         LayoutFieldDelegate* layoutDelegate = nullptr);
  LayoutField* layoutField() { return &m_layoutField; }
  void setHighlighted(bool highlight) override;
  Responder* responder() override { return this; }
  int numberOfSubviews() const override { return 1; }
  View* subviewAtIndex(int index) override { return &m_layoutField; }
  void layoutSubviews(bool force = false) override;
  void didBecomeFirstResponder() override;
  KDSize minimalSizeForOptimalDisplay() const override;
  void drawRect(KDContext* ctx, KDRect rect) const override;

 private:
  constexpr static KDCoordinate k_minimalHeight =
      Metric::ExpressionInputBarMinimalHeight;
  constexpr static KDCoordinate k_maximalHeight = 0.6 * Ion::Display::Height;
  constexpr static KDCoordinate k_margin = Metric::LayoutViewMargin;
  // topMargin has + 1 to look the same as when the separator was included in it
  constexpr static KDCoordinate k_topMargin = k_margin + 1;
  constexpr static KDCoordinate k_separatorThickness =
      Metric::CellSeparatorThickness;
  LayoutField m_layoutField;
  char m_expressionBody[Poincare::PrintFloat::k_maxFloatCharSize];
};

}  // namespace Escher

#endif
