#include "type_controller.h"

#include <apps/i18n.h>
#include <assert.h>
#include <escher/view_controller.h>
#include <ion/events.h>
#include <poincare/print.h>

#include "inference/app.h"
#include "inference/text_helpers.h"
#include "test/hypothesis_controller.h"

using namespace Escher;

namespace Inference {

TypeController::TypeController(StackViewController *parent,
                               HypothesisController *hypothesisController,
                               InputController *inputController,
                               Statistic *statistic)
    : Escher::SelectableListViewController<
          Escher::StandardMemoizedListViewDataSource>(parent),
      m_hypothesisController(hypothesisController),
      m_inputController(inputController),
      m_statistic(statistic) {
  m_selectableListView.setBottomMargin(0);

  // Init selection
  selectRow(0);
}

void TypeController::didBecomeFirstResponder() {
  resetMemoization();
  m_selectableListView.reloadData(true);
}

bool TypeController::handleEvent(Ion::Events::Event event) {
  // canBeActivatedByEvent can be called on any cell with chevron
  if (!m_cells[0].canBeActivatedByEvent(event)) {
    return popFromStackViewControllerOnLeftEvent(event);
  }
  DistributionType type;
  int selRow = selectedRow();
  if (selRow == k_indexOfTTest) {
    type = DistributionType::T;
  } else if (selRow == indexOfZTest()) {
    type = DistributionType::Z;
  } else {
    assert(selRow == k_indexOfPooledTest);
    type = DistributionType::TPooled;
  }
  Escher::ViewController *controller = m_inputController;
  if (m_statistic->hasHypothesisParameters()) {
    controller = m_hypothesisController;
  }
  assert(controller != nullptr);
  m_statistic->initializeDistribution(type);
  stackOpenPage(controller);
  return true;
}

const char *TypeController::title() {
  I18n::Message format = m_statistic->distributionTitle();
  I18n::Message testOrInterval = m_statistic->statisticBasicTitle();
  Poincare::Print::CustomPrintf(m_titleBuffer, sizeof(m_titleBuffer),
                                I18n::translate(format),
                                I18n::translate(testOrInterval));
  return m_titleBuffer;
}

void TypeController::stackOpenPage(Escher::ViewController *nextPage) {
  switch (m_statistic->distributionType()) {
    case DistributionType::T:
      selectRow(k_indexOfTTest);
      break;
    case DistributionType::TPooled:
      selectRow(k_indexOfPooledTest);
      break;
    default:
      assert(m_statistic->distributionType() == DistributionType::Z);
      selectRow(indexOfZTest());
      break;
  }
  ViewController::stackOpenPage(nextPage);
}

int TypeController::numberOfRows() const {
  return m_statistic->numberOfAvailableDistributions();
}

void TypeController::fillCellForRow(Escher::HighlightCell *cell, int row) {
  assert(row <= indexOfZTest());
  I18n::Message message;
  if (row == k_indexOfTTest) {
    message = m_statistic->tDistributionName();
  } else if (row == indexOfZTest()) {
    message = m_statistic->zDistributionName();
  } else {
    assert(row == k_indexOfPooledTest);
    message = m_statistic->tPooledDistributionName();
  }

  static_cast<Escher::MenuCell<Escher::MessageTextView, Escher::EmptyCellWidget,
                               Escher::ChevronView> *>(cell)
      ->label()
      ->setMessage(message);
}

}  // namespace Inference
