#include "list_parameter_controller.h"

#include <apps/i18n.h>
#include <apps/shared/poincare_helpers.h>

#include "../app.h"
#include "list_controller.h"

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Sequence {

ListParameterController::ListParameterController(ListController *listController)
    : Shared::ListParameterController(listController,
                                      I18n::Message::SequenceColor,
                                      I18n::Message::DeleteSequence, this),
      m_initialRankCell(&m_selectableListView, this),
      m_typeParameterController(this, listController, Metric::CommonTopMargin,
                                Metric::CommonRightMargin,
                                Metric::CommonBottomMargin,
                                Metric::CommonLeftMargin) {
  m_typeCell.label()->setMessage(I18n::Message::SequenceType);
  m_initialRankCell.label()->setMessage(I18n::Message::FirstTermIndex);
}

const char *ListParameterController::title() {
  return I18n::translate(I18n::Message::SequenceOptions);
}

bool ListParameterController::textFieldShouldFinishEditing(
    AbstractTextField *textField, Ion::Events::Event event) {
  return event == Ion::Events::Down || event == Ion::Events::Up ||
         MathTextFieldDelegate::textFieldShouldFinishEditing(textField, event);
}

bool ListParameterController::textFieldDidFinishEditing(
    AbstractTextField *textField, const char *text, Ion::Events::Event event) {
  double floatBody = ParseInputtedFloatValue<double>(text);
  if (HasUndefinedValue(floatBody)) {
    return false;
  }
  int index = std::floor(floatBody);
  if (index < 0 || index > Shared::Sequence::k_maxInitialRank) {
    App::app()->displayWarning(I18n::Message::ForbiddenValue);
    return false;
  }
  sequence()->setInitialRank(index);
  App::app()->snapshot()->updateInterval();
  // Invalidate sequence context cache when changing sequence type
  App::app()->localContext()->resetCache();
  m_selectableListView.reloadSelectedCell();
  m_selectableListView.handleEvent(event);
  return true;
}

void ListParameterController::listViewDidChangeSelectionAndDidScroll(
    SelectableListView *l, int previousSelectedRow, KDPoint previousOffset,
    bool withinTemporarySelection) {
  assert(l == &m_selectableListView);
  if (withinTemporarySelection || previousSelectedRow == l->selectedRow()) {
    return;
  }
  if (previousSelectedRow == 1) {
    assert(l->cell(previousSelectedRow) == &m_initialRankCell);
    m_initialRankCell.textField()->setEditing(false);
    App::app()->setFirstResponder(&m_selectableListView);
  }
  if (l->selectedRow() == 1) {
    assert(l->selectedCell() == &m_initialRankCell);
    App::app()->setFirstResponder(&m_initialRankCell);
  }
}

HighlightCell *ListParameterController::cell(int index) {
  assert(0 <= index && index < numberOfRows());
  HighlightCell *const cells[] = {&m_typeCell, &m_initialRankCell,
                                  &m_enableCell, &m_colorCell, &m_deleteCell};
  return cells[index];
}

void ListParameterController::fillCellForRow(HighlightCell *cell, int row) {
  Shared::ListParameterController::fillCellForRow(cell, row);
  if (cell == &m_typeCell && !m_record.isNull()) {
    m_typeCell.subLabel()->setLayout(sequence()->definitionName());
  }
  if (cell == &m_initialRankCell && !m_record.isNull()) {
    if (m_initialRankCell.textField()->isEditing()) {
      return;
    }
    char buffer[Shared::Sequence::k_initialRankNumberOfDigits + 1];
    Poincare::Integer(sequence()->initialRank())
        .serialize(buffer, Shared::Sequence::k_initialRankNumberOfDigits + 1);
    m_initialRankCell.textField()->setText(buffer);
  }
}

bool ListParameterController::handleEvent(Ion::Events::Event event) {
  HighlightCell *cell = selectedCell();
  if (cell == &m_typeCell && m_typeCell.canBeActivatedByEvent(event)) {
    m_typeParameterController.setRecord(m_record);
    static_cast<StackViewController *>(parentResponder())
        ->push(&m_typeParameterController);
    return true;
  }
  if (cell == &m_enableCell && m_enableCell.canBeActivatedByEvent(event)) {
    App::app()->localContext()->resetCache();
    function()->setActive(!function()->isActive());
    m_selectableListView.reloadSelectedCell();
    return true;
  }
  return Shared::ListParameterController::handleEvent(event);
}

}  // namespace Sequence
