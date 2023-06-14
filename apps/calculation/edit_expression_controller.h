#ifndef CALCULATION_EDIT_EXPRESSION_CONTROLLER_H
#define CALCULATION_EDIT_EXPRESSION_CONTROLLER_H

#include <apps/shared/math_field_delegate.h>
#include <poincare/layout.h>

#include "calculation_selectable_table_view.h"
#include "history_controller.h"
#include "layout_field.h"

namespace Calculation {

/* TODO: implement a split view */
class EditExpressionController : public Escher::ViewController,
                                 public Shared::MathFieldDelegate {
 public:
  /* k_layoutBufferMaxSize dictates the size under which the expression being
   * edited can be remembered when the user leaves Calculation. */
  constexpr static int k_layoutBufferMaxSize = 1024;
  /* k_cacheBufferSize is the size of the array to which m_cacheBuffer points.
   * It is used both as a way to buffer expression when pushing them the
   * CalculationStore, and as a storage for the current input when leaving the
   * application. */
  constexpr static int k_cacheBufferSize =
      std::max(k_layoutBufferMaxSize, Constant::MaxSerializedExpressionSize);

  EditExpressionController(
      Escher::Responder* parentResponder,
      Escher::InputEventHandlerDelegate* inputEventHandlerDelegate,
      HistoryController* historyController, CalculationStore* calculationStore);

  /* ViewController */
  Escher::View* view() override { return &m_contentView; }
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;

  /* MathFieldDelegate */
  bool layoutFieldDidReceiveEvent(Escher::LayoutField* layoutField,
                                  Ion::Events::Event event) override;
  bool layoutFieldDidHandleEvent(Escher::LayoutField* layoutField,
                                 bool returnValue,
                                 bool layoutDidChange) override;
  bool layoutFieldDidFinishEditing(Escher::LayoutField* layoutField,
                                   Poincare::Layout layoutR,
                                   Ion::Events::Event event) override;
  void layoutFieldDidAbortEditing(Escher::LayoutField* layoutField) override;
  void layoutFieldDidChangeSize(Escher::LayoutField* layoutField) override;
  bool isAcceptableExpression(Escher::EditableField* field,
                              const Poincare::Expression expression) override;

  void insertTextBody(const char* text);
  void restoreInput();

 private:
  class ContentView : public Escher::View {
   public:
    ContentView(Escher::Responder* parentResponder,
                CalculationSelectableTableView* subview,
                Escher::InputEventHandlerDelegate* inputEventHandlerDelegate,
                Escher::LayoutFieldDelegate* layoutFieldDelegate);
    void reload();
    CalculationSelectableTableView* mainView() { return m_mainView; }
    Escher::LayoutField* layoutField() {
      return m_expressionInputBar.layoutField();
    }

   private:
    int numberOfSubviews() const override { return 2; }
    View* subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    CalculationSelectableTableView* m_mainView;
    ExpressionInputBar m_expressionInputBar;
  };

  void reloadView();
  void clearWorkingBuffer() { m_workingBuffer[0] = 0; }
  bool inputViewDidReceiveEvent(Ion::Events::Event event,
                                bool shouldDuplicateLastCalculation);
  bool inputViewDidHandleEvent(bool returnValue);
  bool inputViewDidFinishEditing(const char* text, Poincare::Layout layoutR);
  bool inputViewDidAbortEditing(const char* text);
  void memoizeInput();

  char m_workingBuffer[k_layoutBufferMaxSize];
  HistoryController* m_historyController;
  CalculationStore* m_calculationStore;
  ContentView m_contentView;
};

}  // namespace Calculation

#endif
