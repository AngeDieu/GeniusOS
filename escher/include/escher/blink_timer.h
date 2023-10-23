#ifndef ESCHER_BLINK_TIMER_H
#define ESCHER_BLINK_TIMER_H

#include <escher/text_cursor_view.h>
#include <escher/timer.h>

namespace Escher {

class BlinkTimer : public Timer {
 public:
  constexpr static uint32_t k_blinkPeriod = 600;
  static_assert(k_blinkPeriod > TickDuration,
                "BlinkTimer period is shorter than one tick.");

  BlinkTimer() : Timer(k_blinkPeriod / TickDuration) {}

  void forceCursorVisible();

 private:
  static TextCursorView* SharedCursor() {
    return TextCursorView::sharedTextCursor;
  }

  bool fire() override;
};

}  // namespace Escher

#endif
