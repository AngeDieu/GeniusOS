#ifndef CALCULATION_HISTORY_CONTROLLER_H
#define CALCULATION_HISTORY_CONTROLLER_H

#include "additional_outputs/complex_list_controller.h"
#include "additional_outputs/function_list_controller.h"
#include "additional_outputs/integer_list_controller.h"
#include "additional_outputs/matrix_list_controller.h"
#include "additional_outputs/rational_list_controller.h"
#include "additional_outputs/scientific_notation_list_controller.h"
#include "additional_outputs/trigonometry_list_controller.h"
#include "additional_outputs/unit_list_controller.h"
#include "additional_outputs/vector_list_controller.h"
#include "calculation_selectable_list_view.h"
#include "calculation_store.h"
#include "history_view_cell.h"

namespace Calculation {

class App;

class HistoryController : public Escher::ViewController,
                          public Escher::ListViewDataSource,
                          public Escher::SelectableListViewDataSource,
                          public Escher::SelectableListViewDelegate,
                          public HistoryViewCellDataSource {
 public:
  HistoryController(EditExpressionController* editExpressionController,
                    CalculationStore* calculationStore);
  Escher::View* view() override { return &m_selectableListView; }
  bool handleEvent(Ion::Events::Event event) override;
  void viewWillAppear() override;
  TELEMETRY_ID("");
  void didBecomeFirstResponder() override;
  void willExitResponderChain(Responder* nextFirstResponder) override;
  void reload();
  int numberOfRows() const override;
  Escher::HighlightCell* reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  void setSelectedSubviewType(SubviewType subviewType, bool sameCell,
                              int previousSelectedRow = -1) override;
  void listViewDidChangeSelectionAndDidScroll(
      Escher::SelectableListView* list, int previousSelectedRow,
      KDPoint previousOffset, bool withinTemporarySelection = false) override;
  void recomputeHistoryCellHeightsIfNeeded() {
    m_calculationStore->recomputeHeightsIfPreferencesHaveChanged(
        Poincare::Preferences::sharedPreferences, HistoryViewCell::Height);
  }

 private:
  KDCoordinate nonMemoizedRowHeight(int row) override;

  int storeIndex(int i) { return numberOfRows() - i - 1; }
  Shared::ExpiringPointer<Calculation> calculationAtIndex(int i);
  bool calculationAtIndexToggles(int index);
  void handleOK();

  constexpr static int k_maxNumberOfDisplayedRows = 5;

  CalculationSelectableListView m_selectableListView;
  HistoryViewCell m_calculationHistory[k_maxNumberOfDisplayedRows];
  CalculationStore* m_calculationStore;
  union UnionController {
    UnionController(EditExpressionController* editExpressionController)
        : m_complexController(editExpressionController) {}
    ~UnionController() { listController()->~ExpressionsListController(); }
    ExpressionsListController* listController() {
      return static_cast<ExpressionsListController*>(&m_complexController);
    }
    ComplexListController m_complexController;
    TrigonometryListController m_trigonometryController;
    UnitListController m_unitController;
    MatrixListController m_matrixController;
    VectorListController m_vectorController;
    FunctionListController m_functionController;
  };
  UnionController m_unionController;
  // These results may be appended to another "main" result
  IntegerListController m_integerController;
  RationalListController m_rationalController;
  ScientificNotationListController m_scientificNotationListController;
};

}  // namespace Calculation

#endif
