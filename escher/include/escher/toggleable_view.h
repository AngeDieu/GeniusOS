#ifndef ESCHER_TOGGLEABLE_VIEW_H
#define ESCHER_TOGGLEABLE_VIEW_H

#include <escher/view.h>

namespace Escher {

class ToggleableView : public View {
 public:
  ToggleableView() : m_state(true) {}
  bool state() const { return m_state; }
  void setState(bool state);

 protected:
  bool m_state;
};

}  // namespace Escher
#endif
