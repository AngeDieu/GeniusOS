#include <escher/blink_timer.h>
#include <escher/text_cursor_view.h>

namespace Escher {

OMG::GlobalBox<TextCursorView> TextCursorView::sharedTextCursor;

void TextCursorView::CursorFieldView::layoutCursorSubview(bool force) {
  if (TextCursorView::sharedTextCursor->isInField(this)) {
    TextCursorView::sharedTextCursor->willMove();
    setChildFrame(TextCursorView::sharedTextCursor, cursorRect(), force);
  }
}

int TextCursorView::CursorFieldView::numberOfSubviews() const {
  return TextCursorView::sharedTextCursor->isInField(this) ? 1 : 0;
}

View* TextCursorView::CursorFieldView::subviewAtIndex(int index) {
  assert(index == numberOfSubviews() - 1 &&
         TextCursorView::sharedTextCursor->isInField(this));
  return TextCursorView::sharedTextCursor;
}

void TextCursorView::drawRect(KDContext* ctx, KDRect rect) const {
  if (m_visible) {
    KDCoordinate height = bounds().height();
    ctx->fillRect(KDRect(0, 0, k_width, height), KDColorBlack);
  }
}

KDSize TextCursorView::minimalSizeForOptimalDisplay() const {
  return KDSize(k_width, 0);
}

void TextCursorView::setInField(TextCursorView::CursorFieldView* field) {
  if (!field) {
    setVisible(false);
    m_field = nullptr;
    return;
  }
  m_field = field;
  m_field->layoutCursorSubview(true);
}

void TextCursorView::willMove() {
  /* Force the cursor to appears when its frame changes. This way, the user does
   * not lose sight of the cursor when moving around.  We need to expose a
   * callback since layoutSubviews is not called when the size is kept. */
  m_visible = true;
  // No need to mark rect as dirty since it will be moved
}

void TextCursorView::setVisible(bool visible) {
  if (visible == m_visible) {
    return;
  }
  m_visible = visible;
  if (visible) {
    markWholeFrameAsDirty();
  } else {
    // Redraw the part under the cursor
    m_field->markAbsoluteRectAsDirty(absoluteFrame());
  }
}

}  // namespace Escher
