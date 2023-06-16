#include "script_name_cell.h"

#include <assert.h>

#include "app.h"

using namespace Escher;

namespace Code {

const char* ScriptNameCell::text() const {
  if (!m_textField.isEditing()) {
    return m_textField.text();
  }
  return nullptr;
}

KDSize ScriptNameCell::minimalSizeForOptimalDisplay() const {
  return m_textField.minimalSizeForOptimalDisplay();
}

void ScriptNameCell::didBecomeFirstResponder() {
  App::app()->setFirstResponder(&m_textField);
}

void ScriptNameCell::updateSubviewsBackgroundAfterChangingState() {
  m_textField.setBackgroundColor(backgroundColor());
}

void ScriptNameCell::layoutSubviews(bool force) {
  KDRect cellBounds = bounds();
  setChildFrame(&m_textField,
                KDRect(cellBounds.x() + k_leftMargin, cellBounds.y(),
                       cellBounds.width() - k_leftMargin, cellBounds.height()),
                force);
}

}  // namespace Code
