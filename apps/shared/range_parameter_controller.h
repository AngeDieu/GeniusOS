#ifndef SHARED_RANGE_PARAMETER_CONTROLLER_H
#define SHARED_RANGE_PARAMETER_CONTROLLER_H

#include <escher/buffer_text_view.h>
#include <escher/button_cell.h>
#include <escher/chevron_view.h>
#include <escher/menu_cell.h>
#include <escher/message_text_view.h>
#include <escher/unequal_view.h>

#include "interactive_curve_view_range.h"
#include "pop_up_controller.h"
#include "single_interactive_curve_view_range_controller.h"

namespace Shared {

class RangeParameterController
    : public Escher::SelectableListViewController<
          Escher::StandardMemoizedListViewDataSource> {
 public:
  RangeParameterController(
      Escher::Responder *parentResponder,
      InteractiveCurveViewRange *interactiveCurveViewRange);

  const char *title() override { return I18n::translate(I18n::Message::Axis); }

  int numberOfRows() const override {
    return displayNormalizeCell() + k_numberOfRangeCells + 1;
  }
  int typeAtRow(int row) const override;
  int reusableCellCount(int type) override;
  Escher::HighlightCell *reusableCell(int index, int type) override;
  KDCoordinate nonMemoizedRowHeight(int row) override;
  void fillCellForRow(Escher::HighlightCell *cell, int row) override;
  KDCoordinate separatorBeforeRow(int row) override;

  void viewWillAppear() override;
  void viewDidDisappear() override;
  bool handleEvent(Ion::Events::Event event) override;

  void setRange(InteractiveCurveViewRange *range);
  Escher::StackViewController *stackController() {
    return static_cast<Escher::StackViewController *>(parentResponder());
  }
  TELEMETRY_ID("Range");

 private:
  constexpr static int k_numberOfRangeCells = 2;
  constexpr static int k_normalizeCellType = 0;
  constexpr static int k_rangeCellType = 1;
  constexpr static int k_okCellType = 2;

  void buttonAction();
  bool displayNormalizeCell() const {
    return !m_tempInteractiveRange.zoomNormalize();
  }

  InteractiveCurveViewRange *m_interactiveRange;
  InteractiveCurveViewRange m_tempInteractiveRange;
  Escher::MenuCell<Escher::MessageTextView, Escher::EmptyCellWidget,
                   Escher::UnequalView>
      m_normalizeCell;
  Escher::MenuCell<Escher::MessageTextView, Escher::OneLineBufferTextView<>,
                   Escher::ChevronView>
      m_rangeCells[k_numberOfRangeCells];
  Escher::ButtonCell m_okButton;
  Shared::MessagePopUpController m_confirmPopUpController;
  SingleInteractiveCurveViewRangeController
      m_singleInteractiveCurveViewRangeController;
};

}  // namespace Shared

#endif
