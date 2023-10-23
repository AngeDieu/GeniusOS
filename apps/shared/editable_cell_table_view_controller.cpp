#include "editable_cell_table_view_controller.h"

#include <assert.h>
#include <escher/even_odd_cell.h>
#include <escher/even_odd_editable_text_cell.h>
#include <escher/stack_view_controller.h>
#include <poincare/print.h>

#include <algorithm>
#include <cmath>

#include "../constant.h"
#include "column_parameter_controller.h"
#include "poincare_helpers.h"
#include "text_field_delegate_app.h"

using namespace Escher;
using namespace Poincare;

namespace Shared {

EditableCellTableViewController::EditableCellTableViewController(
    Responder *parentResponder, Escher::SelectableTableViewDelegate *delegate)
    : TabTableController(parentResponder),
      m_selectableTableView(this, this, this, this, delegate) {}

bool EditableCellTableViewController::textFieldShouldFinishEditing(
    AbstractTextField *textField, Ion::Events::Event event) {
  return TextFieldDelegate::textFieldShouldFinishEditing(textField, event) ||
         (event == Ion::Events::Down && selectedRow() < numberOfRows()) ||
         (event == Ion::Events::Up && selectedRow() > 0) ||
         (event == Ion::Events::Right &&
          (textField->cursorLocation() ==
           textField->draftTextBuffer() + textField->draftTextLength()) &&
          selectedColumn() < numberOfColumns() - 1) ||
         (event == Ion::Events::Left &&
          textField->cursorLocation() == textField->draftTextBuffer() &&
          selectedColumn() > 0);
}

bool EditableCellTableViewController::textFieldDidFinishEditing(
    AbstractTextField *textField, const char *text, Ion::Events::Event event) {
  double floatBody =
      textFieldDelegateApp()->parseInputtedFloatValue<double>(text);
  if (textFieldDelegateApp()->hasUndefinedValue(floatBody)) {
    return false;
  }
  // Save attributes for later use
  int column = selectedColumn();
  int row = selectedRow();
  KDCoordinate rwHeight = rowHeight(row);
  int previousNumberOfElementsInColumn = numberOfElementsInColumn(column);
  if (!checkDataAtLocation(floatBody, column, row)) {
    Container::activeApp()->displayWarning(I18n::Message::ForbiddenValue);
    return false;
  }
  if (!setDataAtLocation(floatBody, column, row)) {
    // Storage memory error
    return false;
  }

  didChangeCell(column, row);
  updateSizeMemoizationForRow(row, rwHeight);
  // Reload other cells
  if (previousNumberOfElementsInColumn < numberOfElementsInColumn(column)) {
    // Reload the whole table, if a value was appended.
    updateSizeMemoizationForRow(row + 1, 0);  // update total height
    selectableTableView()->reloadData();
  } else {
    assert(previousNumberOfElementsInColumn ==
           numberOfElementsInColumn(column));
    reloadEditedCell(column, row);
  }

  if (event == Ion::Events::EXE || event == Ion::Events::OK) {
    selectableTableView()->selectCellAtLocation(column, row + 1);
  } else {
    selectableTableView()->handleEvent(event);
  }
  return true;
}

int EditableCellTableViewController::numberOfRows() const {
  int numberOfModelElements = 0;
  for (int i = 0; i < numberOfColumns(); i++) {
    numberOfModelElements =
        std::max(numberOfModelElements, numberOfElementsInColumn(i));
  }
  return 1 + numberOfModelElements +
         (numberOfModelElements < maxNumberOfElements());
}

void EditableCellTableViewController::fillCellForLocationWithDisplayMode(
    HighlightCell *cell, int column, int row,
    Preferences::PrintFloatMode floatDisplayMode) {
  static_cast<EvenOddCell *>(cell)->setEven(row % 2 == 0);
  if (row == 0) {
    setTitleCellText(cell, column);
    setTitleCellStyle(cell, column);
    return;
  }
  // The cell is editable
  if (cellAtLocationIsEditable(column, row)) {
    AbstractEvenOddEditableTextCell *myEditableValueCell =
        static_cast<AbstractEvenOddEditableTextCell *>(cell);
    constexpr int bufferSize = PrintFloat::charSizeForFloatsWithPrecision(
        AbstractEvenOddBufferTextCell::k_defaultPrecision);
    char buffer[bufferSize];
    // Special case 1: last row and NaN
    if (row == numberOfElementsInColumn(column) + 1 ||
        std::isnan(dataAtLocation(column, row))) {
      buffer[0] = 0;
    } else {
      PoincareHelpers::ConvertFloatToTextWithDisplayMode<double>(
          dataAtLocation(column, row), buffer, bufferSize,
          AbstractEvenOddBufferTextCell::k_defaultPrecision, floatDisplayMode);
    }
    myEditableValueCell->editableTextCell()->textField()->setText(buffer);
  }
}

void EditableCellTableViewController::didBecomeFirstResponder() {
  if (selectedRow() >= 0) {
    int selRow = selectedRow();
    selRow = selRow >= numberOfRows() ? numberOfRows() - 1 : selRow;
    int selColumn = selectedColumn();
    selColumn =
        selColumn >= numberOfColumns() ? numberOfColumns() - 1 : selColumn;
    selectCellAtLocation(selColumn, selRow);
    TabTableController::didBecomeFirstResponder();
  }
}

void EditableCellTableViewController::viewWillAppear() {
  TabTableController::viewWillAppear();
  if (selectedRow() == -1) {
    selectCellAtLocation(0, 1);
  } else {
    int selRow = selectedRow();
    selRow = selRow >= numberOfRows() ? numberOfRows() - 1 : selRow;
    int selColumn = selectedColumn();
    selColumn =
        selColumn >= numberOfColumns() ? numberOfColumns() - 1 : selColumn;
    selectCellAtLocation(selColumn, selRow);
  }
}

bool EditableCellTableViewController::handleEvent(Ion::Events::Event event) {
  if ((event == Ion::Events::Backspace && selectedRow() == 0) ||
      event == Ion::Events::Clear) {
    presentClearSelectedColumnPopupIfClearable();
    return true;
  }
  if ((event == Ion::Events::OK || event == Ion::Events::EXE) &&
      selectedRow() == 0) {
    SelectableViewController *controller = columnParameterController();
    ColumnParameters *parameters = columnParameters();
    if (controller != nullptr) {
      parameters
          ->initializeColumnParameters();  // Always initialize before pushing
      /* Reset here because we want to stay on the same row if we come from a
       * submenu. */
      controller->selectRow(0);
      stackController()->push(controller);
    }
    return true;
  }
  return false;
}

}  // namespace Shared
