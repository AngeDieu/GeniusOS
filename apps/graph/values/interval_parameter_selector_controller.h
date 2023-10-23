#ifndef GRAPH_INTERVAL_PARAMETER_SELECTOR_CONTROLLER
#define GRAPH_INTERVAL_PARAMETER_SELECTOR_CONTROLLER

#include <apps/shared/interval_parameter_controller.h>
#include <escher/chevron_view.h>
#include <escher/menu_cell.h>
#include <escher/message_text_view.h>
#include <escher/selectable_list_view_controller.h>

#include "../../shared/continuous_function.h"

namespace Graph {

class IntervalParameterSelectorController
    : public Escher::SelectableListViewController<
          Escher::SimpleListViewDataSource> {
 public:
  IntervalParameterSelectorController();
  const char* title() override;
  void viewWillAppear() override { m_selectableListView.reloadData(); }
  bool handleEvent(Ion::Events::Event event) override;
  int numberOfRows() const override;
  int reusableCellCount() const override { return k_numberOfSymbolTypes; }
  Escher::HighlightCell* reusableCell(int index) override;
  void fillCellForRow(Escher::HighlightCell* cell, int row) override;
  void setStartEndMessages(
      Shared::IntervalParameterController* controller,
      Shared::ContinuousFunctionProperties::SymbolType symbolType);

 private:
  constexpr static size_t k_numberOfSymbolTypes =
      Shared::ContinuousFunctionProperties::k_numberOfSymbolTypes;
  Shared::ContinuousFunctionProperties::SymbolType symbolTypeAtRow(int j) const;
  I18n::Message messageForType(
      Shared::ContinuousFunctionProperties::SymbolType symbolType);
  Escher::MenuCell<Escher::MessageTextView, Escher::EmptyCellWidget,
                   Escher::ChevronView>
      m_intervalParameterCell[k_numberOfSymbolTypes];
};

}  // namespace Graph

#endif
