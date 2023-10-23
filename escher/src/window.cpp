#include <escher/window.h>
#include <ion.h>
extern "C" {
#include <assert.h>
}

namespace Escher {

void Window::redraw(bool force) {
  if (force) {
    markWholeFrameAsDirty();
  }
  Ion::Display::waitForVBlank();
  View::redraw(bounds());
}

void Window::setContentView(View* contentView) {
  m_contentView = contentView;
  markWholeFrameAsDirty();
  layoutSubviews();
}

int Window::numberOfSubviews() const {
  return (m_contentView == nullptr ? 0 : 1);
}

View* Window::subviewAtIndex(int index) {
  assert(m_contentView != nullptr && index == 0);
  return m_contentView;
}

void Window::layoutSubviews(bool force) {
  if (m_contentView != nullptr) {
    setChildFrame(m_contentView, bounds(), force);
  }
}

#if ESCHER_VIEW_LOGGING
const char* Window::className() const { return "Window"; }

#endif

}  // namespace Escher
