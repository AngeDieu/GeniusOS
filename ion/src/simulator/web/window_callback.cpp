#include <emscripten.h>

#include "../shared/window.h"

namespace Ion {
namespace Simulator {
namespace Window {

void didRefresh() {
  /* Notify JS that the display has been refreshed.
   * This gives us a chance to mirror the display in fullscreen mode. */
  // clang-format off
  EM_ASM(if (typeof Module.onDisplayRefresh === "function") { Module.onDisplayRefresh(); });
  // clang-format on
}

}  // namespace Window
}  // namespace Simulator
}  // namespace Ion
