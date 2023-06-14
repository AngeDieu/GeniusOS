#ifndef SOLVER_LIST_CONTROLLER_H
#define SOLVER_LIST_CONTROLLER_H

#include <apps/i18n.h>
#include <apps/shared/expression_model_list_controller.h>
#include <escher/button_row_controller.h>
#include <escher/editable_expression_model_cell.h>
#include <escher/even_odd_expression_cell.h>

#include "equation_list_view.h"
#include "equation_models_parameter_controller.h"
#include "equation_store.h"

namespace Solver {

class ListController : public Shared::ExpressionModelListController,
                       public Escher::ButtonRowDelegate {
 public:
  ListController(Escher::Responder* parentResponder,
                 EquationStore* equationStore,
                 Escher::ButtonRowController* footer);
  /* ButtonRowDelegate */
  int numberOfButtons(
      Escher::ButtonRowController::Position position) const override;
  Escher::AbstractButtonCell* buttonAtIndex(
      int index, Escher::ButtonRowController::Position position) const override;
  /* ListViewDataSource */
  Escher::HighlightCell* reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  /* Responder */
  bool handleEvent(Ion::Events::Event event) override;
  void didBecomeFirstResponder() override;
  void didEnterResponderChain(
      Escher::Responder* previousFirstResponder) override;
  /* ViewController */
  Escher::View* view() override { return &m_equationListView; }
  TELEMETRY_ID("List");
  /* MathFieldDelegate */
  bool layoutFieldDidReceiveEvent(Escher::LayoutField* layoutField,
                                  Ion::Events::Event event) override;
  bool layoutFieldDidFinishEditing(Escher::LayoutField* layoutField,
                                   Poincare::Layout layout,
                                   Ion::Events::Event event) override;
  void layoutFieldDidChangeSize(Escher::LayoutField* layoutField) override;
  void layoutFieldDidAbortEditing(Escher::LayoutField* layoutField) override;
  bool isAcceptableExpression(const Poincare::Expression expression) override;
  /* ExpressionModelListController */
  void editExpression(Ion::Events::Event event) override;
  /* Specific to Solver */
  void resolveEquations();

  Escher::LayoutField* layoutField() override {
    return m_editableCell.layoutField();
  }

 private:
  constexpr static int k_maxNumberOfRows =
      Escher::Metric::MinimalNumberOfScrollableRowsToFillDisplayHeight(
          Escher::Metric::StoreRowHeight,
          Escher::Metric::ButtonRowEmbossedStyleHeightLarge);
  Escher::SelectableListView* selectableListView() override;
  void reloadButtonMessage();
  void addModel() override;
  bool removeModelRow(Ion::Storage::Record record) override;
  void reloadBrace();
  EquationStore* modelStore() const override;
  Escher::StackViewController* stackController() const;

  // ListViewDataSource
  KDCoordinate nonMemoizedRowHeight(int row) override;

  EquationListView m_equationListView;
  Escher::EvenOddExpressionCell m_expressionCells[k_maxNumberOfRows];
  Escher::EditableExpressionModelCell m_editableCell;
  Escher::AbstractButtonCell m_resolveButton;
  EquationModelsParameterController m_modelsParameterController;
  Escher::StackViewController m_modelsStackController;
};

}  // namespace Solver

#endif
