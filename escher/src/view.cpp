#include <escher/view.h>
#include <kandinsky/ion_context.h>

extern "C" {
#include <assert.h>
}

namespace Escher {

void View::markRectAsDirty(KDRect rect) {
  markAbsoluteRectAsDirty(rect.translatedBy(m_frame.origin()));
}

void View::markAbsoluteRectAsDirty(KDRect rect) {
  /* Intersect with m_frame before unioning to avoid KDCoordinate overflow. */
  m_dirtyRect = m_dirtyRect.intersectedWith(m_frame).unionedWith(
      rect.intersectedWith(m_frame));
}

KDRect View::redraw(KDRect rect, KDRect forceRedrawRect) {
  /* View::redraw recursively redraws the rectangle 'rect' of the view and all
   * its subviews.
   * To optimize the function, we redraw only the union of the current dirty
   * rectangle with a rectangle forced to be redrawn (forceRedrawRect). This
   * rectangle is initially empty and recursively expands by unioning with the
   * rectangles that are redrawn. This process handles the case when several
   * sister views are overlapping (provided that the sister views are indexed in
   * the right order).
   */

  /* First, for the current view, the rectangle to redraw is the union of the
   * dirty rectangle and the rectangle forced to be redrawn. The rectangle to
   * redraw must also be included in the current view bounds and in the
   * rectangle rect. */
  KDRect visibleRect = rect.intersectedWith(m_frame);
  KDRect rectNeedingRedraw =
      visibleRect.intersectedWith(m_dirtyRect)
          .unionedWith(forceRedrawRect.intersectedWith(m_frame));

  // This redraws the rectNeedingRedraw calling drawRect.
  if (!rectNeedingRedraw.isEmpty()) {
    KDPoint absOrigin = absoluteOrigin();
    KDContext *ctx = KDIonContext::SharedContext;
    ctx->setOrigin(absOrigin);
    ctx->setClippingRect(rectNeedingRedraw);
    drawRect(ctx, rectNeedingRedraw.relativeTo(m_frame.origin()));
  }
  // This initializes the area that has been redrawn.
  KDRect redrawnArea = rectNeedingRedraw;

  // Then, let's recursively draw our children over ourself
  uint8_t subviewsNumber = numberOfSubviews();
  for (uint8_t i = 0; i < subviewsNumber; i++) {
    View *subview = subviewAtIndex(i);
    if (subview == nullptr) {
      continue;
    }

    /* We redraw the current subview by passing the rectangle previously redrawn
     * (by the parent view or previous sister views) as forced to be redraw. */
    KDRect subviewRedrawnArea = subview->redraw(visibleRect, redrawnArea);

    // We expand the redrawn area to include the area just drawn.
    redrawnArea = redrawnArea.unionedWith(subviewRedrawnArea);
  }
  // Eventually, mark that we don't need to be redrawn
  m_dirtyRect = KDRectZero;

  // The function returns the total area that have been redrawn.
  return redrawnArea;
}

void View::setSize(KDSize size) {
  setFrame(KDRect(m_frame.origin(), size), false);
}

void View::setChildFrame(View *child, KDRect frame, bool force) {
  /* We will move the child. This will leave a blank spot in this view where it
   * previously was. At this point, we know that the only area that needs to be
   * redrawn in the superview is the old frame minus the part covered by the new
   * frame.
   * Check first if m_dirtyRect == m_frame. If it is, it's useless to
   * compute previousFrame since everything is already dirty.
   * WARNING: When this->setFrame is called, m_frame changes which makes
   * relativeChildFrame return a wrong value. Fortunately, in this case,
   * m_dirtyRect == m_frame so we can avoid calling relativeChildFrame. */
  if (m_dirtyRect != m_frame) {
    KDRect previousFrame = relativeChildFrame(child);
    markRectAsDirty(previousFrame.differencedWith(frame));
  }
  child->setFrame(frame.translatedBy(m_frame.origin()), force);
}

void View::setFrame(KDRect frame, bool force) {
  if (!force) {
    if (frame == m_frame) {
      return;
    }
    /* This optimization avoids calling layoutSubviews each time the frame is
     * just translated. The problem is that subviewAtIndex is sometimes a bit
     * unreliable since some views have different subviews depending on their
     * current state. (thats why we can't assert that `child` is a subview of
     * `this` in `setChildFrame`). So this might cause some problems in the
     * future and might need a bit of rework.
     * Also m_frame.isEmpty() must be escaped because layoutSubviews is also
     * escaped when m_frame.isEmpty(). If not, a subview could be translated
     * while not having been properly re-layouted earlier. */
    if (frame.size() == m_frame.size() && !m_frame.isEmpty()) {
      translate(frame.origin().relativeTo(m_frame.origin()));
      markWholeFrameAsDirty();
      return;
    }
  }

  /* CAUTION: This code is not resilient to multiple consecutive setFrame()
   * calls without intermediate redraw() calls. */
  m_frame = frame;

  /* Now that we have moved, we have also dirtied our new absolute frame.
   * There are two ways to declare this, which are semantically equivalent: we
   * can either mark an area of our superview as dirty, or mark our whole frame
   * as dirty. We pick the second option because it is more efficient. */
  markWholeFrameAsDirty();

  if (!m_frame.isEmpty()) {
    layoutSubviews(force);
  }
}

void View::translate(KDPoint delta) {
  m_frame = m_frame.translatedBy(delta);
  uint8_t subviewsNumber = numberOfSubviews();
  for (uint8_t i = 0; i < subviewsNumber; i++) {
    View *subview = subviewAtIndex(i);
    if (subview == nullptr) {
      continue;
    }
    subview->translate(delta);
  }
}

KDPoint View::pointFromPointInView(View *view, KDPoint point) {
  return point.translatedBy(relativeChildOrigin(view));
}

KDRect View::bounds() const { return m_frame.movedTo(KDPointZero); }

#if ESCHER_VIEW_LOGGING
const char *View::className() const { return "View"; }

void View::logAttributes(std::ostream &os) const {
  os << " address=\"" << this << "\"";
  os << " frame=\"" << m_frame.x() << "," << m_frame.y() << ","
     << m_frame.width() << "," << m_frame.height() << "\"";
}

std::ostream &operator<<(std::ostream &os, View &view) {
  // TODO find something better than this static variable, like a custom stream
  static int indentColumn = 0;
  constexpr static int bufferSize = 200;
  ;
  char indent[bufferSize];
  for (int i = 0; i < indentColumn; i++) {
    indent[i] = ' ';
  }
  indent[indentColumn] = 0;
  os << indent << "<" << view.className();
  view.logAttributes(os);
  if (view.numberOfSubviews()) {
    os << ">\n";
    for (int i = 0; i < view.numberOfSubviews(); i++) {
      indentColumn += 2;
      os << *view.subviewAtIndex(i) << '\n';
      indentColumn -= 2;
    }
    os << indent << "</" << view.className() << ">";
  } else {
    os << "/>";
  }
  return os;
}

void View::log() const {
  // TODO << should use a const View but it requires a const subview
  std::cout << *const_cast<View *>(this);
}
#endif

}  // namespace Escher
