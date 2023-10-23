#include "interval_parameter_selector_controller.h"

#include <apps/i18n.h>
#include <assert.h>

#include "../../shared/interval_parameter_controller.h"
#include "../app.h"

using namespace Escher;

namespace Graph {

IntervalParameterSelectorController::IntervalParameterSelectorController()
    : SelectableListViewController(nullptr) {}

const char *IntervalParameterSelectorController::title() {
  return I18n::translate(I18n::Message::IntervalSet);
}

bool IntervalParameterSelectorController::handleEvent(
    Ion::Events::Event event) {
  // canBeActivatedByEvent can be called on any cell with chevron
  if (m_intervalParameterCell[0].canBeActivatedByEvent(event)) {
    StackViewController *stack = (StackViewController *)parentResponder();
    Shared::IntervalParameterController *controller =
        App::app()->valuesController()->intervalParameterController();
    Shared::ContinuousFunctionProperties::SymbolType symbolType =
        symbolTypeAtRow(selectedRow());
    controller->setTitle(messageForType(symbolType));
    setStartEndMessages(controller, symbolType);
    controller->setInterval(App::app()->intervalForSymbolType(symbolType));
    stack->push(controller);
    return true;
  }
  return false;
}

int IntervalParameterSelectorController::numberOfRows() const {
  int rowCount = 0;
  size_t symbolTypeIndex = 0;
  Shared::ContinuousFunctionProperties::SymbolType symbolType;
  while (symbolTypeIndex < k_numberOfSymbolTypes) {
    symbolType = static_cast<Shared::ContinuousFunctionProperties::SymbolType>(
        symbolTypeIndex);
    bool symbolTypeIsShown =
        App::app()->functionStore()->numberOfActiveFunctionsInTableOfSymbolType(
            symbolType) > 0;
    rowCount += symbolTypeIsShown;
    symbolTypeIndex++;
  }
  return rowCount;
}

HighlightCell *IntervalParameterSelectorController::reusableCell(int index) {
  assert(0 <= index && index < reusableCellCount());
  return m_intervalParameterCell + index;
}

void IntervalParameterSelectorController::fillCellForRow(HighlightCell *cell,
                                                         int row) {
  assert(0 <= row && row < numberOfRows());
  Shared::ContinuousFunctionProperties::SymbolType symbolType =
      symbolTypeAtRow(row);
  static_cast<MenuCell<MessageTextView, EmptyCellWidget, ChevronView> *>(cell)
      ->label()
      ->setMessage(messageForType(symbolType));
}

Shared::ContinuousFunctionProperties::SymbolType
IntervalParameterSelectorController::symbolTypeAtRow(int j) const {
  int rowCount = 0;
  size_t symbolTypeIndex = 0;
  Shared::ContinuousFunctionProperties::SymbolType symbolType;
  do {
    symbolType = static_cast<Shared::ContinuousFunctionProperties::SymbolType>(
        symbolTypeIndex);
    bool symbolTypeIsShown =
        App::app()->functionStore()->numberOfActiveFunctionsInTableOfSymbolType(
            symbolType) > 0;
    if (symbolTypeIsShown && rowCount == j) {
      break;
    }
    rowCount += symbolTypeIsShown;
  } while (symbolTypeIndex++ < k_numberOfSymbolTypes);
  assert(rowCount == j);
  return symbolType;
}

I18n::Message IntervalParameterSelectorController::messageForType(
    Shared::ContinuousFunctionProperties::SymbolType symbolType) {
  switch (symbolType) {
    case Shared::ContinuousFunctionProperties::SymbolType::X:
      return I18n::Message::IntervalX;
    case Shared::ContinuousFunctionProperties::SymbolType::T:
      return I18n::Message::IntervalT;
    case Shared::ContinuousFunctionProperties::SymbolType::Radius:
      return I18n::Message::IntervalR;
    default:
      assert(symbolType ==
             Shared::ContinuousFunctionProperties::SymbolType::Theta);
      return I18n::Message::IntervalTheta;
  }
}

void IntervalParameterSelectorController::setStartEndMessages(
    Shared::IntervalParameterController *controller,
    Shared::ContinuousFunctionProperties::SymbolType symbolType) {
  switch (symbolType) {
    case Shared::ContinuousFunctionProperties::SymbolType::X:
      controller->setStartEndMessages(I18n::Message::XStart,
                                      I18n::Message::XEnd);
      return;
    case Shared::ContinuousFunctionProperties::SymbolType::T:
      controller->setStartEndMessages(I18n::Message::TStart,
                                      I18n::Message::TEnd);
      return;
    case Shared::ContinuousFunctionProperties::SymbolType::Radius:
      controller->setStartEndMessages(I18n::Message::RStart,
                                      I18n::Message::REnd);
      return;
    default:
      assert(symbolType ==
             Shared::ContinuousFunctionProperties::SymbolType::Theta);
      controller->setStartEndMessages(I18n::Message::ThetaStart,
                                      I18n::Message::ThetaEnd);
  }
}

}  // namespace Graph
