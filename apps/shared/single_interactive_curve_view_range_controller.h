#ifndef SHARED_SINGLE_INTERACTIVE_CURVE_VIEW_RANGE_CONTROLLER_H
#define SHARED_SINGLE_INTERACTIVE_CURVE_VIEW_RANGE_CONTROLLER_H

#include <apps/shared/single_range_controller.h>

#include "interactive_curve_view_range.h"

namespace Shared {

class SingleInteractiveCurveViewRangeController : public SingleRangeController {
 public:
  SingleInteractiveCurveViewRangeController(
      Escher::Responder* parentResponder,
      Escher::InputEventHandlerDelegate* inputEventHandlerDelegate,
      InteractiveCurveViewRange* interactiveCurveViewRange,
      MessagePopUpController* confirmPopUpController);

  const char* title() override {
    return I18n::translate(m_editXRange ? I18n::Message::ValuesOfX
                                        : I18n::Message::ValuesOfY);
  }

  bool editXRange() const { return m_editXRange; }
  void setEditXRange(bool editXRange);

 private:
  I18n::Message parameterMessage(int index) const override {
    return index == 0 ? I18n::Message::Minimum : I18n::Message::Maximum;
  }
  bool parametersAreDifferent() override;
  void extractParameters() override;
  void setAutoRange() override;
  float limit() const override { return InteractiveCurveViewRange::k_maxFloat; }
  void confirmParameters() override;
  void pop(bool onConfirmation) override { stackController()->pop(); }

  InteractiveCurveViewRange* m_range;
  // m_secondaryRangeParam is only used when activating xAuto while yAuto is on.
  Poincare::Range1D m_secondaryRangeParam;
  bool m_editXRange;
};

}  // namespace Shared

#endif
