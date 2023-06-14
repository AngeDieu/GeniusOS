#include "term_sum_controller.h"

#include <poincare/code_point_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/vertical_offset_layout.h>

#include <cmath>

#include "../app.h"

extern "C" {
#include <assert.h>
#include <stdlib.h>
}

using namespace Shared;
using namespace Poincare;
using namespace Escher;

namespace Sequence {

TermSumController::TermSumController(Responder* parentResponder,
                                     Escher::BoxesDelegate* boxesDelegate,
                                     GraphView* graphView,
                                     CurveViewRange* graphRange,
                                     CurveViewCursor* cursor)
    : SumGraphController(parentResponder, boxesDelegate, graphView, graphRange,
                         cursor) {}

const char* TermSumController::title() {
  return I18n::translate(I18n::Message::TermSum);
}

bool TermSumController::moveCursorHorizontallyToPosition(double position) {
  if (position < 0.0) {
    return false;
  }
  return SumGraphController::moveCursorHorizontallyToPosition(
      std::round(position));
}

I18n::Message TermSumController::legendMessageAtStep(Step step) {
  switch (step) {
    case Step::FirstParameter:
      return I18n::Message::SelectFirstTerm;
    case Step::SecondParameter:
      return I18n::Message::SelectLastTerm;
    default:
      return I18n::Message::Default;
  }
}

double TermSumController::cursorNextStep(double x,
                                         OMG::HorizontalDirection direction) {
  double delta = direction.isRight() ? 1.0 : -1.0;
  return std::round(m_cursor->x() + delta);
}

Layout TermSumController::createFunctionLayout() {
  ExpiringPointer<Shared::Sequence> sequence =
      App::app()->functionStore()->modelForRecord(selectedRecord());
  return sequence->nameLayout();
}

}  // namespace Sequence
