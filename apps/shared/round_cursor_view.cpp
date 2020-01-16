#include "round_cursor_view.h"

namespace Shared {

static KDColor s_cursorWorkingBuffer[Dots::LargeDotDiameter*Dots::LargeDotDiameter];

void RoundCursorView::drawRect(KDContext * ctx, KDRect rect) const {
  KDRect r = bounds();
  ctx->getPixels(r, m_underneathPixelBuffer);
  m_underneathPixelBufferLoaded = true;
  ctx->blendRectWithMask(r, m_color, (const uint8_t *)Dots::LargeDotMask, s_cursorWorkingBuffer);
}

KDSize RoundCursorView::minimalSizeForOptimalDisplay() const {
  return KDSize(k_cursorSize, k_cursorSize);
}

void RoundCursorView::setColor(KDColor color) {
  m_color = color;
  eraseCursorIfPossible();
  markRectAsDirty(bounds());
}

void RoundCursorView::setCursorFrame(KDRect f, bool force) {
#if GRAPH_CURSOR_SPEEDUP
  /* TODO This is quite dirty (we are out of the dirty tracking and we assume
   * the cursor is the upmost view) but it works well. */
  if (m_frame == f && !force) {
    return;
  }
  /* We want to avoid drawing the curve just because the cursor has been
   * repositioned, as it is very slow for non cartesian curves.*/
  if (eraseCursorIfPossible()) {
    // Set the frame
    m_frame = f;
    markRectAsDirty(bounds());
    return;
  }
#endif
  CursorView::setCursorFrame(f, force);
}

#ifdef GRAPH_CURSOR_SPEEDUP
bool RoundCursorView::eraseCursorIfPossible() {
  if (!m_underneathPixelBufferLoaded) {
    return false;
  }
  const KDRect currentFrame = absoluteVisibleFrame();
  if (currentFrame.isEmpty()) {
    return false;
  }
  // Erase the cursor
  KDContext * ctx = KDIonContext::sharedContext();
  ctx->setOrigin(currentFrame.origin());
  ctx->setClippingRect(currentFrame);
  ctx->fillRectWithPixels(KDRect(0,0,k_cursorSize, k_cursorSize), m_underneathPixelBuffer, s_cursorWorkingBuffer);
  // TODO Restore the context to previous values?
  return true;
}
#endif

}
