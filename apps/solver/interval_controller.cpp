#include "interval_controller.h"

#include <apps/i18n.h>
#include <assert.h>
#include <string.h>

#include "app.h"

using namespace Escher;

namespace Solver {

IntervalController::IntervalController(Responder *parentResponder)
    : FloatParameterController<double>(parentResponder),
      m_instructions(I18n::Message::ApproximateSolutionIntervalInstruction,
                     k_messageFormat) {
  setTopView(&m_instructions);
  m_okButton.setMessage(I18n::Message::ResolveEquation);
  for (int i = 0; i < k_maxNumberOfCells; i++) {
    m_intervalCell[i].setParentResponder(&m_selectableListView);
    m_intervalCell[i].setDelegate(this);
  }
}

const char *IntervalController::title() {
  return I18n::translate(I18n::Message::SearchInverval);
}

int IntervalController::numberOfRows() const { return k_maxNumberOfCells + 1; }

void IntervalController::fillCellForRow(HighlightCell *cell, int row) {
  if (typeAtRow(row) == k_buttonCellType) {
    return;
  }
  assert(typeAtRow(row) == k_parameterCellType);
  assert(0 <= row && row < k_maxNumberOfCells);
  I18n::Message labels[k_maxNumberOfCells] = {I18n::Message::XMin,
                                              I18n::Message::XMax};
  static_cast<MenuCellWithEditableText<MessageTextView> *>(cell)
      ->label()
      ->setMessage(labels[row]);
  FloatParameterController::fillCellForRow(cell, row);
}

HighlightCell *IntervalController::reusableParameterCell(int index, int type) {
  assert(index >= 0);
  assert(index < 2);
  return &m_intervalCell[index];
}

TextField *IntervalController::textFieldOfCellAtIndex(HighlightCell *cell,
                                                      int index) {
  assert(typeAtRow(index) == k_parameterCellType);
  return static_cast<MenuCellWithEditableText<MessageTextView> *>(cell)
      ->textField();
}

int IntervalController::reusableParameterCellCount(int type) {
  return k_maxNumberOfCells;
}

double IntervalController::parameterAtIndex(int index) {
  SystemOfEquations *system = App::app()->system();
  return index == 0 ? system->approximateResolutionMinimum()
                    : system->approximateResolutionMaximum();
}

bool IntervalController::setParameterAtIndex(int parameterIndex, double f) {
  SystemOfEquations *system = App::app()->system();
  parameterIndex == 0 ? system->setApproximateResolutionMinimum(f)
                      : system->setApproximateResolutionMaximum(f);
  return true;
}

bool IntervalController::textFieldDidFinishEditing(AbstractTextField *textField,
                                                   Ion::Events::Event event) {
  if (FloatParameterController::textFieldDidFinishEditing(textField, event)) {
    resetSizeMemoization();
    m_selectableListView.reloadData();
    return true;
  }
  return false;
}

void IntervalController::buttonAction() {
  StackViewController *stack = stackController();
  App::app()->system()->approximateSolve(App::app()->localContext());
  stack->push(App::app()->solutionsController());
}

}  // namespace Solver
