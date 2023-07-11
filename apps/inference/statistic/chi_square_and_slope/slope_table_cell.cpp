#include "slope_table_cell.h"

#include <escher/palette.h>

#include "inference/app.h"
#include "inference/statistic/chi_square_and_slope/input_goodness_controller.h"
#include "input_slope_controller.h"

using namespace Escher;

namespace Inference {

SlopeTableCell::SlopeTableCell(Responder *parentResponder, Statistic *statistic,
                               Poincare::Context *parentContext,
                               InputSlopeController *inputSlopeController)
    : DoubleColumnTableCell(parentResponder, statistic),
      StoreColumnHelper(this, parentContext, this),
      m_inputSlopeController(inputSlopeController) {
  for (int i = 0; i < k_maxNumberOfColumns; i++) {
    m_header[i].setColor(Escher::Palette::Red);
    m_header[i].setEven(true);
    m_header[i].setFont(KDFont::Size::Small);
  }
}

void SlopeTableCell::willAppear() {
  for (int i = 0; i < k_maxNumberOfColumns; i++) {
    /* We delayed filling the column names X1, Y1 to ensure that the underlying
     * model was a valid DoublePairStore. */
    fillColumnName(i, const_cast<char *>(m_header[i].text()));
  }
  m_selectableTableView.reloadData(false);
}

InputViewController *SlopeTableCell::inputViewController() {
  return App::app()->inputViewController();
}

void SlopeTableCell::reload() { recomputeDimensions(true); }

CategoricalController *SlopeTableCell::categoricalController() {
  return m_inputSlopeController;
}

}  // namespace Inference
