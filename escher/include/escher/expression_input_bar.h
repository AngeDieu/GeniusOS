#ifndef ESCHER_EXPRESSION_INPUT_BAR_H
#define ESCHER_EXPRESSION_INPUT_BAR_H

#include <escher/layout_field.h>
#include <escher/metric.h>
#include <escher/solid_color_view.h>

namespace Escher {

class AbstractExpressionInputBar : public View {
 public:
  AbstractExpressionInputBar();

  void layoutSubviews(bool force) override;
  int numberOfSubviews() const override { return 2; }
  View* subviewAtIndex(int index) override;
  KDSize minimalSizeForOptimalDisplay() const override;
  virtual const LayoutField* layoutField() const = 0;
  virtual LayoutField* layoutField() = 0;

 protected:
  constexpr static KDMargins k_margins = KDMargins(Metric::LayoutViewMargin);

 private:
  constexpr static KDCoordinate k_minimalHeight =
      Metric::ExpressionInputBarMinimalHeight;
  constexpr static KDCoordinate k_maximalHeight = 0.6 * Ion::Display::Height;
  constexpr static KDCoordinate k_separatorThickness =
      Metric::CellSeparatorThickness;
  KDCoordinate inputViewHeight() const;
  SolidColorView m_line;
};

template <class T>
class TemplatedExpressionInputBar : public AbstractExpressionInputBar {
 public:
  TemplatedExpressionInputBar(Responder* parentResponder,
                              LayoutFieldDelegate* layoutFieldDelegate)
      : AbstractExpressionInputBar(),
        m_field(parentResponder, layoutFieldDelegate) {
    layoutField()->setMargins(k_margins);
  }

  const LayoutField* layoutField() const override { return &m_field; }
  LayoutField* layoutField() override { return &m_field; }

 private:
  T m_field;
};

using ExpressionInputBar = TemplatedExpressionInputBar<LayoutField>;

}  // namespace Escher

#endif
