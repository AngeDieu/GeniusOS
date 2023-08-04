#include "history_controller.h"

#include <apps/shared/expression_display_permissions.h>
#include <assert.h>
#include <poincare/circuit_breaker_checkpoint.h>
#include <poincare/exception_checkpoint.h>
#include <poincare/trigonometry.h>

#include "app.h"

using namespace Shared;
using namespace Poincare;
using namespace Escher;

namespace Calculation {

HistoryController::HistoryController(
    EditExpressionController *editExpressionController,
    CalculationStore *calculationStore)
    : ViewController(editExpressionController),
      m_selectableListView(this, this, this, this),
      m_calculationStore(calculationStore),
      m_unionController(editExpressionController),
      m_integerController(editExpressionController),
      m_rationalController(editExpressionController),
      m_scientificNotationListController(editExpressionController) {
  for (int i = 0; i < k_maxNumberOfDisplayedRows; i++) {
    m_calculationHistory[i].setParentResponder(&m_selectableListView);
    m_calculationHistory[i].setDataSource(this);
  }
}

void HistoryController::reload() {
  // Ensure that the total height never overflows KDCoordinate
  while (m_selectableListView.minimalSizeForOptimalDisplay().height() >=
         KDCOORDINATE_MAX - 2 * HistoryViewCell::k_maxCellHeight) {
    m_calculationStore->deleteCalculationAtIndex(
        m_calculationStore->numberOfCalculations() - 1);
  }
  /* When reloading, we might not used anymore cell that hold previous layouts.
   * We clean them all before reloading their content to avoid taking extra
   * useless space in the Poincare pool. */
  for (int i = 0; i < k_maxNumberOfDisplayedRows; i++) {
    m_calculationHistory[i].resetMemoization();
  }

  m_selectableListView.reloadData();
  /* TODO
   * Replace the following by selectCellAtLocation in order to avoid laying out
   * the table view twice.
   */
  if (numberOfRows() > 0) {
    m_selectableListView.scrollToBottom();
    /* Force to reload last added cell (hide the burger and exact output if
     * necessary) */
    listViewDidChangeSelectionAndDidScroll(&m_selectableListView, -1,
                                           KDPointZero);
  }
}

void HistoryController::viewWillAppear() {
  ViewController::viewWillAppear();
  reload();
}

void HistoryController::didBecomeFirstResponder() {
  selectRow(numberOfRows() - 1);
  App::app()->setFirstResponder(&m_selectableListView);
}

void HistoryController::willExitResponderChain(Responder *nextFirstResponder) {
  if (nextFirstResponder == nullptr) {
    return;
  }
  if (nextFirstResponder == parentResponder()) {
    m_selectableListView.deselectTable();
  }
}

static bool isIntegerInput(Expression e) {
  return (e.type() == ExpressionNode::Type::BasedInteger ||
          (e.type() == ExpressionNode::Type::Opposite &&
           isIntegerInput(e.childAtIndex(0))));
}

static bool isFractionInput(Expression e) {
  if (e.type() == ExpressionNode::Type::Opposite) {
    return isFractionInput(e.childAtIndex(0));
  }
  if (e.type() != ExpressionNode::Type::Division) {
    return false;
  }
  Expression num = e.childAtIndex(0);
  Expression den = e.childAtIndex(1);
  return isIntegerInput(num) && isIntegerInput(den);
}

static void breakableSetExpressionInListController(
    ExpressionsListController **vc, Poincare::Expression exact,
    Poincare::Expression approximate) {
  if (*vc == nullptr) {
    return;
  }
  CircuitBreakerCheckpoint checkpoint(
      Ion::CircuitBreaker::CheckpointType::Back);
  if (CircuitBreakerRun(checkpoint)) {
    Expression exactClone =
        !exact.isUninitialized() ? exact.clone() : Expression();
    Expression approximateClone =
        !approximate.isUninitialized() ? approximate.clone() : Expression();
    (*vc)->setExactAndApproximateExpression(exactClone, approximateClone);
  } else {
    (*vc)->tidy();
    *vc = nullptr;
  }
}

bool HistoryController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Down) {
    m_selectableListView.deselectTable();
    App::app()->setFirstResponder(parentResponder());
    return true;
  }
  if (event == Ion::Events::Up) {
    return true;
  }
  Context *context = App::app()->localContext();
  if (event == Ion::Events::Backspace) {
    int focusRow = selectedRow();
    SubviewType subviewType = m_selectedSubviewType;
    m_selectableListView.deselectTable();
    m_calculationStore->deleteCalculationAtIndex(storeIndex(focusRow));
    reload();
    if (numberOfRows() == 0) {
      App::app()->setFirstResponder(parentResponder());
      return true;
    }
    m_selectableListView.selectCell(focusRow > 0 ? focusRow - 1 : 0);
    setSelectedSubviewType(subviewType, false);
    return true;
  }
  if (event == Ion::Events::Clear) {
    m_selectableListView.deselectTable();
    m_calculationStore->deleteAll();
    reload();
    App::app()->setFirstResponder(parentResponder());
    return true;
  }
  if (event == Ion::Events::Back) {
    m_selectableListView.deselectTable();
    App::app()->setFirstResponder(parentResponder());
    return true;
  }
  if (event != Ion::Events::OK && event != Ion::Events::EXE) {
    return false;
  }
  int focusRow = selectedRow();
  HistoryViewCell *selectedCell =
      (HistoryViewCell *)m_selectableListView.selectedCell();
  SubviewType subviewType = m_selectedSubviewType;
  EditExpressionController *editController =
      (EditExpressionController *)parentResponder();
  if (subviewType == SubviewType::Input) {
    m_selectableListView.deselectTable();
    editController->insertTextBody(calculationAtIndex(focusRow)->inputText());
  } else if (subviewType == SubviewType::Output) {
    m_selectableListView.deselectTable();
    Shared::ExpiringPointer<Calculation> calculation =
        calculationAtIndex(focusRow);
    ScrollableTwoLayoutsView::SubviewPosition outputSubviewPosition =
        selectedCell->outputView()->selectedSubviewPosition();
    if (outputSubviewPosition ==
            ScrollableTwoLayoutsView::SubviewPosition::Right &&
        calculation->displayOutput(context) !=
            Calculation::DisplayOutput::ExactOnly) {
      editController->insertTextBody(calculation->approximateOutputText(
          Calculation::NumberOfSignificantDigits::Maximal));
    } else {
      assert(calculation->displayOutput(context) !=
             Calculation::DisplayOutput::ApproximateOnly);
      editController->insertTextBody(calculation->exactOutputText());
    }
  } else {
    assert(subviewType == SubviewType::Ellipsis);
    /* Only function results can be chained (with integer or rational).
     * TODO: Refactor to avoid writing an if for each parent * child. */
    ExpressionsListController *vc = nullptr;
    Calculation::AdditionalInformations additionalInformations =
        selectedCell->additionalInformations();
    ExpiringPointer<Calculation> focusCalculation =
        calculationAtIndex(focusRow);
    assert(focusCalculation->displayOutput(context) !=
           Calculation::DisplayOutput::ExactOnly);
    Expression i = focusCalculation->input();
    Expression a = focusCalculation->approximateOutput(
        Calculation::NumberOfSignificantDigits::Maximal);
    Expression e = focusCalculation->displayOutput(context) !=
                           Calculation::DisplayOutput::ApproximateOnly
                       ? focusCalculation->exactOutput()
                       : a;
    if (additionalInformations.complex || additionalInformations.unit ||
        additionalInformations.vector || additionalInformations.matrix ||
        additionalInformations.directTrigonometry ||
        additionalInformations.inverseTrigonometry) {
      // The main controllers have to be initialized before use
      m_unionController.~UnionController();
      vc = m_unionController.listController();
      if (additionalInformations.complex) {
        new (&m_unionController) ComplexListController(editController);
      } else if (additionalInformations.unit) {
        new (&m_unionController) UnitListController(editController);
      } else if (additionalInformations.vector) {
        new (&m_unionController) VectorListController(editController);
      } else if (additionalInformations.matrix) {
        new (&m_unionController) MatrixListController(editController);
      } else if (additionalInformations.directTrigonometry ||
                 additionalInformations.inverseTrigonometry) {
        new (&m_unionController) TrigonometryListController(editController);
        if (additionalInformations.directTrigonometry) {
          // Find the angle
          if (Trigonometry::isDirectTrigonometryFunction(e)) {
            e = e.childAtIndex(0);
          } else {
            assert(Trigonometry::isDirectTrigonometryFunction(i));
            e = i.childAtIndex(0);
          }
          /* The approximation of the new e is not yet computed and will be
           * by the controller. */
          a = Expression();
        }
      }
    } else if (additionalInformations.integer) {
      vc = &m_integerController;
    } else if (additionalInformations.rational) {
      if (isFractionInput(i)) {
        e = i;
      }
      vc = &m_rationalController;
    }

    breakableSetExpressionInListController(&vc, e, a);

    if (additionalInformations.function) {
      assert(vc == nullptr || vc == &m_integerController ||
             vc == &m_rationalController);
      ChainableExpressionsListController *tail =
          static_cast<ChainableExpressionsListController *>(vc);
      m_unionController.~UnionController();
      new (&m_unionController) FunctionListController(editController);
      m_unionController.m_functionController.setTail(tail);
      vc = m_unionController.listController();
      breakableSetExpressionInListController(&vc, i, a);
    } else if (additionalInformations.scientificNotation) {
      // TODO function and scientific ?
      assert(vc == nullptr || vc == &m_integerController ||
             vc == &m_rationalController);
      ChainableExpressionsListController *tail =
          static_cast<ChainableExpressionsListController *>(vc);
      m_scientificNotationListController.setTail(tail);
      vc = &m_scientificNotationListController;
      breakableSetExpressionInListController(&vc, e, a);
    }

    if (vc) {
      assert(vc->numberOfRows() > 0);
      App::app()->displayModalViewController(vc, 0.f, 0.f,
                                             Metric::PopUpMarginsNoBottom);
    }
  }
  return true;
}

Shared::ExpiringPointer<Calculation> HistoryController::calculationAtIndex(
    int i) {
  return m_calculationStore->calculationAtIndex(storeIndex(i));
}

void HistoryController::listViewDidChangeSelectionAndDidScroll(
    SelectableListView *list, int previousSelectedRow, KDPoint previousOffset,
    bool withinTemporarySelection) {
  assert(list == &m_selectableListView);
  m_selectableListView.didChangeSelectionAndDidScroll();
  if (withinTemporarySelection || previousSelectedRow == selectedRow()) {
    return;
  }
  if (previousSelectedRow == -1) {
    setSelectedSubviewType(SubviewType::Output, false, previousSelectedRow);
  } else if (selectedRow() == -1) {
    setSelectedSubviewType(SubviewType::None, false, previousSelectedRow);
  } else {
    HistoryViewCell *selectedCell = (HistoryViewCell *)(list->selectedCell());
    SubviewType nextSelectedSubviewType = m_selectedSubviewType;
    if (selectedCell && !selectedCell->displaysSingleLine()) {
      nextSelectedSubviewType = previousSelectedRow < selectedRow()
                                    ? SubviewType::Input
                                    : SubviewType::Output;
    }
    setSelectedSubviewType(nextSelectedSubviewType, false, previousSelectedRow);
  }
}

int HistoryController::numberOfRows() const {
  return m_calculationStore->numberOfCalculations();
};

HighlightCell *HistoryController::reusableCell(int index, int type) {
  assert(type == 0);
  assert(index >= 0);
  assert(index < k_maxNumberOfDisplayedRows);
  return &m_calculationHistory[index];
}

int HistoryController::reusableCellCount(int type) {
  assert(type == 0);
  return k_maxNumberOfDisplayedRows;
}

void HistoryController::fillCellForRow(HighlightCell *cell, int row) {
  HistoryViewCell *myCell = static_cast<HistoryViewCell *>(cell);
  Poincare::Context *context = App::app()->localContext();
  myCell->setCalculation(
      calculationAtIndex(row).pointer(),
      row == selectedRow() && m_selectedSubviewType == SubviewType::Output,
      context);
  myCell->setEven(row % 2 == 0);
  myCell->reloadSubviewHighlight();
}

KDCoordinate HistoryController::nonMemoizedRowHeight(int row) {
  if (row >= m_calculationStore->numberOfCalculations()) {
    return 0;
  }
  Shared::ExpiringPointer<Calculation> calculation = calculationAtIndex(row);
  bool expanded =
      row == selectedRow() && m_selectedSubviewType == SubviewType::Output;
  return calculation->height(expanded);
}

bool HistoryController::calculationAtIndexToggles(int index) {
  Context *context = App::app()->localContext();
  return index >= 0 && index < m_calculationStore->numberOfCalculations() &&
         calculationAtIndex(index)->displayOutput(context) ==
             Calculation::DisplayOutput::ExactAndApproximateToggle;
}

void HistoryController::setSelectedSubviewType(SubviewType subviewType,
                                               bool sameCell,
                                               int previousSelectedRow) {
  // Avoid selecting non-displayed ellipsis
  HistoryViewCell *selectedCell =
      static_cast<HistoryViewCell *>(m_selectableListView.selectedCell());
  if (subviewType == SubviewType::Ellipsis && selectedCell &&
      selectedCell->additionalInformations().isEmpty()) {
    subviewType = SubviewType::Output;
  }
  SubviewType previousSubviewType = m_selectedSubviewType;
  m_selectedSubviewType = subviewType;
  /* We need to notify the whole table that the selection changed if it
   * involves the selection/deselection of an output. Indeed, only them can
   * trigger change in the displayed expressions. */

  /* If the selection change triggers the toggling of the outputs, we update
   * the whole table as the height of the selected cell row might have changed.
   */
  if ((subviewType == SubviewType::Output ||
       previousSubviewType == SubviewType::Output) &&
      (calculationAtIndexToggles(selectedRow()) ||
       calculationAtIndexToggles(previousSelectedRow))) {
    m_selectableListView.reloadData(false);
  }

  /* It might be necessary to scroll to the subviewType if the cell overflows
   * the screen */
  if (selectedRow() >= 0) {
    m_selectableListView.scrollToSubviewOfTypeOfCellAtRow(
        subviewType, m_selectableListView.selectedRow());
  }
  /* Refill the selected cell and the previous selected cell because cells
   * repartition might have changed */
  selectedCell =
      static_cast<HistoryViewCell *>(m_selectableListView.selectedCell());
  HistoryViewCell *previousSelectedCell = static_cast<HistoryViewCell *>(
      m_selectableListView.cell(previousSelectedRow));
  /* 'reloadData' calls 'fillCellForRow' for each cell while the table
   * has been deselected. To reload the expanded cell, we call one more time
   * 'fillCellForRow' but once the right cell has been selected. */
  if (selectedCell) {
    fillCellForRow(selectedCell, selectedRow());
  }

  previousSubviewType = sameCell ? previousSubviewType : SubviewType::None;
  if (selectedCell) {
    selectedCell->reloadSubviewHighlight();
    selectedCell->cellDidSelectSubview(subviewType, previousSubviewType);
    App::app()->setFirstResponder(selectedCell, true);
  }
  if (previousSelectedCell) {
    previousSelectedCell->cellDidSelectSubview(SubviewType::Input);
  }
}

}  // namespace Calculation
