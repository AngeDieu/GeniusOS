#include "plot_view_cursors.h"

using namespace Poincare;

namespace Shared {
namespace PlotPolicy {

KDRect WithCursor::cursorFrame(AbstractPlotView* plotView) {
  assert(plotView && m_cursor);
  if (!m_cursorView || !(plotView->hasFocus() && std::isfinite(m_cursor->x()) &&
                         std::isfinite(m_cursor->y()))) {
    return KDRectZero;
  }
  KDSize size = m_cursorView->minimalSizeForOptimalDisplay();
  Coordinate2D<float> p = plotView->floatToPixel2D(
      Coordinate2D<float>(m_cursor->x(), m_cursor->y()));

  /* If the cursor is not visible, put its frame to zero, because it might be
   * very far out of the visible frame and thus later overflow KDCoordinate.
   * The "2" factor is a big safety margin. */
  constexpr int maxCursorPixel = KDCOORDINATE_MAX / 2;
  // Assert we are not removing visible cursors
  static_assert((Ion::Display::Width * 2 < maxCursorPixel) &&
                    (Ion::Display::Height * 2 < maxCursorPixel),
                "maxCursorPixel should be bigger");
  if (std::fabs(p.x()) > maxCursorPixel || std::fabs(p.y()) > maxCursorPixel) {
    return KDRectZero;
  }

  KDCoordinate px = std::round(p.x());
  px -= (size.width() - 1) / 2;
  KDCoordinate py = std::round(p.y());
  py -= (size.height() - 1) / 2;
  if (size.height() == 0) {
    // The cursor is supposed to take up all the available vertical space.
    KDCoordinate plotHeight =
        plotView->bounds().height() -
        (plotView->bannerView()
             ? plotView->bannerView()->minimalSizeForOptimalDisplay().height()
             : 0);
    return KDRect(px, 0, size.width(), plotHeight);
  }
  return KDRect(px, py, size);
}

}  // namespace PlotPolicy
}  // namespace Shared
