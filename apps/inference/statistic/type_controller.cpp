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
    : SelectableCellListPage(parent),
      m_hypothesisController(hypothesisController),
      m_inputController(inputController),
      m_statistic(statistic) {
  m_selectableListView.setBottomMargin(0);
  typedCell(k_indexOfTTest)
      ->label()
      ->setMessage(m_statistic->tDistributionName());
  typedCell(k_indexOfPooledTest)
      ->label()
      ->setMessage(m_statistic->tPooledDistributionName());
  typedCell(k_indexOfZTest)
      ->label()
      ->setMessage(m_statistic->zDistributionName());

  // Init selection
  selectRow(0);
}

void TypeController::didBecomeFirstResponder() {
  resetSizeMemoization();
  m_selectableListView.reloadData(true);
}

bool TypeController::handleEvent(Ion::Events::Event event) {
  // canBeActivatedByEvent can be called on any cell with chevron
  if (!typedCell(0)->canBeActivatedByEvent(event)) {
    return popFromStackViewControllerOnLeftEvent(event);
  }
  DistributionType type;
  int selRow = selectedRow();
  if (selRow == k_indexOfTTest) {
    type = DistributionType::T;
  } else if (selRow == k_indexOfZTest) {
    type = DistributionType::Z;
  } else {
    assert(selRow == k_indexOfPooledTest);
    type = DistributionType::TPooled;
  }
  ViewController *controller = m_inputController;
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

void TypeController::stackOpenPage(ViewController *nextPage) {
  switch (m_statistic->distributionType()) {
    case DistributionType::T:
      selectRow(k_indexOfTTest);
      break;
    case DistributionType::TPooled:
      selectRow(k_indexOfPooledTest);
      break;
    default:
      assert(m_statistic->distributionType() == DistributionType::Z);
      selectRow(k_indexOfZTest);
      break;
  }
  ViewController::stackOpenPage(nextPage);
}

void TypeController::viewWillAppear() {
  typedCell(k_indexOfPooledTest)
      ->setVisible(m_statistic->numberOfAvailableDistributions() ==
                   numberOfRows());
}

}  // namespace Inference
