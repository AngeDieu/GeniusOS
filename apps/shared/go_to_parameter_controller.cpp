#include "go_to_parameter_controller.h"

#include <assert.h>

#include "interactive_curve_view_controller.h"

using namespace Escher;

namespace Shared {

GoToParameterController::GoToParameterController(
    Responder *parentResponder, InteractiveCurveViewRange *graphRange,
    CurveViewCursor *cursor)
    : FloatParameterController<double>(parentResponder),
      m_cursor(cursor),
      m_graphRange(graphRange),
      /* m_tempParameter can be called when computing size before having
       * displayed anything. Set it to a dummy value that will be overriden in
       * viewWillAppear() anyway. */
      m_tempParameter(0.0),
      m_parameterCell(&m_selectableListView, this) {}

HighlightCell *GoToParameterController::reusableParameterCell(int index,
                                                              int type) {
  assert(index == 0);
  return &m_parameterCell;
}

TextField *GoToParameterController::textFieldOfCellAtIndex(
    Escher::HighlightCell *cell, int index) {
  assert(typeAtRow(index) == k_parameterCellType);
  return m_parameterCell.textField();
}

bool GoToParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Left) {
    stackController()->pop();
    return true;
  }
  return false;
}

KDCoordinate GoToParameterController::nonMemoizedRowHeight(int row) {
  int type = typeAtRow(row);
  if (type == k_parameterCellType) {
    return m_parameterCell.minimalSizeForOptimalDisplay().height();
  }
  assert(type == k_buttonCellType);
  return Shared::FloatParameterController<double>::nonMemoizedRowHeight(row);
}

void GoToParameterController::viewWillAppear() {
  // Initialize m_tempParameter to the extracted value.
  setParameterAtIndex(0, extractParameterAtIndex(0));
  FloatParameterController::viewWillAppear();
}

bool GoToParameterController::setParameterAtIndex(int parameterIndex,
                                                  double f) {
  assert(parameterIndex == 0);
  m_tempParameter = f;
  return true;
}

void GoToParameterController::buttonAction() {
  // Update parameter value to m_tempParameter, and proceed if value is valid
  if (confirmParameterAtIndex(0, m_tempParameter)) {
    StackViewController *stack = (StackViewController *)parentResponder();
    stack->popUntilDepth(
        InteractiveCurveViewController::k_graphControllerStackDepth, true);
  }
}

}  // namespace Shared
