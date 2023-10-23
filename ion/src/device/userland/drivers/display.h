#ifndef ION_DEVICE_USERLAND_DRIVERS_DISPLAY_H
#define ION_DEVICE_USERLAND_DRIVERS_DISPLAY_H

#include <kandinsky/color.h>
#include <kandinsky/font.h>
#include <kandinsky/glyph.h>
#include <kandinsky/point.h>

namespace Ion {
namespace Display {

/* WARNING: Do not change signature of this function since it's exposed to
 * external apps */
void drawString(const char* text, KDPoint point, bool largeFont,
                KDColor textColor, KDColor backgroundColor);

}  // namespace Display
}  // namespace Ion

#endif
