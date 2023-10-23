#include "range_parameter_controller.h"

#include "poincare_helpers.h"

using namespace Escher;
using namespace Poincare;

namespace Shared {

// RangeParameterController

RangeParameterController::RangeParameterController(
    Responder *parentResponder,
    InputEventHandlerDelegate *inputEventHandlerDelegate,
    InteractiveCurveViewRange *interactiveRange)
    : SelectableListViewController(parentResponder),
      m_interactiveRange(interactiveRange),
      m_tempInteractiveRange(*interactiveRange),
      m_okButton(
          &m_selectableListView, I18n::Message::Ok,
          Invocation::Builder<RangeParameterController>(
              [](RangeParameterController *parameterController, void *sender) {
                parameterController->buttonAction();
                return true;
              },
              this)),
      m_confirmPopUpController(Invocation::Builder<RangeParameterController>(
          [](RangeParameterController *controller, void *sender) {
            controller->stackController()->pop();
            return true;
          },
          this)),
      m_singleInteractiveCurveViewRangeController(
          parentResponder, inputEventHandlerDelegate, &m_tempInteractiveRange,
          &m_confirmPopUpController) {
  m_normalizeCell.label()->setMessage(I18n::Message::MakeOrthonormal);
  m_rangeCells[0].label()->setMessage(I18n::Message::ValuesOfX);
  m_rangeCells[1].label()->setMessage(I18n::Message::ValuesOfY);
}

int RangeParameterController::typeAtRow(int row) const {
  int types[] = {k_normalizeCellType, k_rangeCellType, k_rangeCellType,
                 k_okCellType};
  return types[row + !displayNormalizeCell()];
}

int RangeParameterController::reusableCellCount(int type) {
  if (type == k_rangeCellType) {
    return k_numberOfRangeCells;
  } else {
    return 1;
  }
}

HighlightCell *RangeParameterController::reusableCell(int index, int type) {
  if (type == k_normalizeCellType) {
    assert(index == 0);
    return &m_normalizeCell;
  } else if (type == k_rangeCellType) {
    assert(index < k_numberOfRangeCells);
    return m_rangeCells + index;
  } else {
    assert(type == k_okCellType);
    assert(index == 0);
    return &m_okButton;
  }
}

KDCoordinate RangeParameterController::nonMemoizedRowHeight(int row) {
  HighlightCell *cells[] = {&m_normalizeCell, m_rangeCells, m_rangeCells + 1,
                            &m_okButton};
  assert(row < numberOfRows());
  return protectedNonMemoizedRowHeight(cells[row + !displayNormalizeCell()],
                                       row);
}

void RangeParameterController::fillCellForRow(HighlightCell *cell, int row) {
  if (typeAtRow(row) == k_rangeCellType) {
    float min, max;
    bool isAuto = false;
    int i =
        static_cast<
            MenuCell<MessageTextView, OneLineBufferTextView<>, ChevronView> *>(
            cell) -
        m_rangeCells;
    if (i == 0) {
      if (m_tempInteractiveRange.xAuto()) {
        isAuto = true;
      } else {
        min = m_tempInteractiveRange.xMin();
        max = m_tempInteractiveRange.xMax();
      }
    } else {
      assert(i == 1);
      if (m_tempInteractiveRange.yAuto()) {
        isAuto = true;
      } else {
        min = m_tempInteractiveRange.yMin();
        max = m_tempInteractiveRange.yMax();
      }
    }
    constexpr int precision = Preferences::VeryLargeNumberOfSignificantDigits;
    constexpr int bufferSize =
        2 * PrintFloat::charSizeForFloatsWithPrecision(precision) + 4;
    char buffer[bufferSize];
    if (isAuto) {
      strlcpy(buffer, I18n::translate(I18n::Message::DefaultSetting),
              bufferSize);
    } else {
      int numberOfChars = PoincareHelpers::ConvertFloatToTextWithDisplayMode(
          min, buffer, bufferSize, precision,
          Preferences::PrintFloatMode::Decimal);
      buffer[numberOfChars++] = ' ';
      buffer[numberOfChars++] = ';';
      buffer[numberOfChars++] = ' ';
      numberOfChars += PoincareHelpers::ConvertFloatToTextWithDisplayMode(
          max, buffer + numberOfChars, bufferSize - numberOfChars, precision,
          Preferences::PrintFloatMode::Decimal);
      buffer[numberOfChars++] = '\0';
    }
    m_rangeCells[i].subLabel()->setText(buffer);
  }
}

KDCoordinate RangeParameterController::separatorBeforeRow(int row) {
  return (displayNormalizeCell() && row == 1) || typeAtRow(row) == k_okCellType
             ? k_defaultRowSeparator
             : 0;
}

void RangeParameterController::viewWillAppear() {
  ViewController::viewWillAppear();
  if (selectedRow() == -1) {
    selectCell(0);
  } else {
    /* If the table has not been deselected, it means we come from the
     * SingleRangeController. */
    int row =
        (m_singleInteractiveCurveViewRangeController.editXRange() ? 0 : 1) +
        displayNormalizeCell();
    selectCell(row);
  }
  resetMemoization();
  m_selectableListView.reloadData();
}

void RangeParameterController::viewDidDisappear() {
  if (!stackController()) {
    m_selectableListView.deselectTable();
  }
}

bool RangeParameterController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back &&
      (m_interactiveRange->rangeChecksum() !=
           m_tempInteractiveRange.rangeChecksum() ||
       m_interactiveRange->xAuto() != m_tempInteractiveRange.xAuto() ||
       m_interactiveRange->yAuto() != m_tempInteractiveRange.yAuto())) {
    // Open pop-up to confirm discarding values
    m_confirmPopUpController.presentModally();
    return true;
  }
  if (displayNormalizeCell() && selectedRow() == 0 &&
      m_normalizeCell.canBeActivatedByEvent(event)) {
    m_normalizeCell.setHighlighted(false);
    m_tempInteractiveRange.normalize();
    buttonAction();
    return true;
  }
  int index = selectedRow() - displayNormalizeCell();
  if (index >= 0 && index < k_numberOfRangeCells &&
      m_rangeCells[index].canBeActivatedByEvent(event)) {
    assert(typeAtRow(selectedRow()) == k_rangeCellType);
    m_singleInteractiveCurveViewRangeController.setEditXRange(index == 0);
    stackController()->push(&m_singleInteractiveCurveViewRangeController);
    return true;
  }
  return false;
}

void RangeParameterController::setRange(InteractiveCurveViewRange *range) {
  m_interactiveRange = range;
  m_tempInteractiveRange = *range;
}

void RangeParameterController::buttonAction() {
  /* Deselect the table before denormalizing, as it would mess up the index by
   * adding a new row. */
  m_selectableListView.deselectTable();

  /* Use setZoomAuto to refresh the Auto button on the graph. */
  m_interactiveRange->setZoomAuto(m_tempInteractiveRange.zoomAuto());
  m_interactiveRange->setZoomNormalize(m_tempInteractiveRange.zoomNormalize());
  *m_interactiveRange = m_tempInteractiveRange;

  stackController()->pop();
}

}  // namespace Shared
