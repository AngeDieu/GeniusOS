#include <assert.h>
#include <escher/i18n.h>
#include <escher/modal_view_empty_controller.h>

namespace Escher {

// ModalViewEmptyController::ModalViewEmptyView

void ModalViewEmptyController::ModalViewEmptyView::initMessageViews() {
  const int numberOfMessageViews = numberOfMessageTextViews();
  for (int i = 0; i < numberOfMessageViews; i++) {
    MessageTextView *message = messageTextViewAtIndex(i);
    message->setFont(k_font);
    message->setBackgroundColor(k_backgroundColor);
    float verticalAlignment = KDGlyph::k_alignCenter;
    if (i == 0) {
      verticalAlignment = KDGlyph::k_alignBottom;
    } else if (i == numberOfMessageViews - 1) {
      verticalAlignment = KDGlyph::k_alignTop;
    }
    message->setAlignment(KDGlyph::k_alignCenter, verticalAlignment);
  }
}

void ModalViewEmptyController::ModalViewEmptyView::setMessages(
    I18n::Message *message) {
  const int numberOfMessageViews = numberOfMessageTextViews();
  for (int i = 0; i < numberOfMessageViews; i++) {
    messageTextViewAtIndex(i)->setMessage(message[i]);
  }
}

void ModalViewEmptyController::ModalViewEmptyView::drawRect(KDContext *ctx,
                                                            KDRect rect) const {
  ctx->fillRect(bounds(), k_backgroundColor);
  drawBorderOfRect(ctx, bounds(), Palette::GrayBright);
}

int ModalViewEmptyController::ModalViewEmptyView::numberOfSubviews() const {
  return numberOfMessageTextViews() + hasLayoutView();
}

void ModalViewEmptyController::ModalViewEmptyView::reload() {
  layoutSubviews();
}

View *ModalViewEmptyController::ModalViewEmptyView::subviewAtIndex(int index) {
  if (hasLayoutView()) {
    if (index == k_layoutViewRow) {
      return layoutView();
    }
    return messageTextViewAtIndex(index + (index < k_layoutViewRow ? 0 : -1));
  }
  return messageTextViewAtIndex(index);
}

void ModalViewEmptyController::ModalViewEmptyView::layoutSubviews(bool force) {
  const int numberOfMessageViews = numberOfMessageTextViews();
  const bool hasLayout = hasLayoutView();
  KDCoordinate width = bounds().width() - 2 * k_separatorThickness;
  KDCoordinate height = bounds().height() - 2 * k_separatorThickness;
  KDCoordinate textHeight = KDFont::GlyphHeight(k_font);
  KDCoordinate layoutHeight =
      hasLayout ? layoutView()->minimalSizeForOptimalDisplay().height() : 0;
  KDCoordinate margin =
      (height - numberOfMessageViews * textHeight - layoutHeight) / 2;
  if (hasLayout) {
    setChildFrame(
        layoutView(),
        KDRect(k_separatorThickness,
               k_separatorThickness + margin + k_layoutViewRow * textHeight,
               width, layoutHeight),
        force);
  }
  KDCoordinate currentHeight = k_separatorThickness;
  for (uint8_t i = 0; i < numberOfMessageViews; i++) {
    if (hasLayout && i == k_layoutViewRow) {
      currentHeight += layoutHeight;
    }
    KDCoordinate h = (i == 0 || i == numberOfMessageViews - 1)
                         ? textHeight + margin
                         : textHeight;
    setChildFrame(messageTextViewAtIndex(i),
                  KDRect(k_separatorThickness, currentHeight, width, h), force);
    currentHeight += h;
  }
}

// ModalViewEmptyController

void ModalViewEmptyController::setMessages(I18n::Message *messages) {
  static_cast<ModalViewEmptyView *>(view())->setMessages(messages);
}

}  // namespace Escher
