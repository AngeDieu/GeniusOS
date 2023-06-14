#include "editable_function_cell.h"

#include <escher/palette.h>

using namespace Escher;

namespace Graph {

EditableFunctionCell::EditableFunctionCell(
    Escher::Responder* parentResponder,
    Escher::BoxesDelegate* boxesDelegate,
    Escher::LayoutFieldDelegate* layoutFieldDelegate)
    : AbstractFunctionCell(),
      m_layoutField(parentResponder, boxesDelegate, layoutFieldDelegate) {
  // We set a dummy message for the height computation
  m_messageTextView.setMessage(I18n::Message::UnhandledType);
  m_layoutField.setLeftMargin(Metric::EditableExpressionAdditionalMargin);
}

void EditableFunctionCell::layoutSubviews(bool force) {
  setChildFrame(&m_ellipsisView,
                KDRect(bounds().width() - k_parametersColumnWidth, 0,
                       k_parametersColumnWidth, bounds().height()),
                force);
  KDCoordinate leftMargin = k_colorIndicatorThickness + k_expressionMargin;
  KDCoordinate rightMargin = k_expressionMargin + k_parametersColumnWidth;
  KDCoordinate availableWidth = bounds().width() - leftMargin - rightMargin;
  setChildFrame(&m_layoutField,
                KDRect(leftMargin, 0, availableWidth, bounds().height()),
                force);
  setChildFrame(&m_messageTextView, KDRectZero, force);
}

void EditableFunctionCell::updateSubviewsBackgroundAfterChangingState() {
  m_expressionBackground = backgroundColor();
}

}  // namespace Graph
