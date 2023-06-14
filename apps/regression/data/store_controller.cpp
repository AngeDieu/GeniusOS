#include "store_controller.h"

#include <assert.h>

#include "../app.h"

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace Regression {

StoreController::StoreController(Responder* parentResponder,
                                 Escher::BoxesDelegate* boxesDelegate,
                                 Store* store, ButtonRowController* header,
                                 Context* parentContext)
    : Shared::StoreController(parentResponder, boxesDelegate, store, header,
                              parentContext),
      m_storeParameterController(this, this) {}

void StoreController::clearSelectedColumn() {
  int series = m_store->seriesAtColumn(selectedColumn());
  m_store->deleteColumn(series, m_store->relativeColumn(selectedColumn()));
  selectCellAtLocation(selectedColumn(), 1);
  resetMemoizedFormulasOfEmptyColumns(series);
}

InputViewController* StoreController::inputViewController() {
  return App::app()->inputViewController();
}

}  // namespace Regression
