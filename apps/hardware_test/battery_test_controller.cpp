#include "battery_test_controller.h"

#include <apps/shared/poincare_helpers.h>

#include "app.h"
extern "C" {
#include <assert.h>
}
#include <ion/post_and_hardware_tests.h>
#include <poincare/preferences.h>
#include <poincare/print.h>
#include <poincare/print_float.h>

using namespace Poincare;
using namespace Shared;
using namespace Escher;

namespace HardwareTest {

View* BatteryTestController::view() { return &m_view; }

bool BatteryTestController::handleEvent(Ion::Events::Event event) {
  // Do not handle OnOff event to let the apps container redraw the screen
  if (event == Ion::Events::OnOff) {
    return false;
  }
  if (event == Ion::Events::OK) {
    if (strcmp(m_view.batteryStateTextView()->text(), k_batteryOKText) == 0) {
      // Handled in WizardViewController
      return false;
    }
  }
  updateBatteryState(Ion::Battery::voltage(), Ion::Battery::isCharging());
  return true;
}

void BatteryTestController::viewWillAppear() {
  bool batteryOK = Ion::POSTAndHardwareTests::BatteryOK();
  const char* text = batteryOK ? k_batteryOKText : k_batteryNeedChargingText;
  KDColor color = batteryOK ? KDColorGreen : KDColorRed;
  m_view.setColor(color);
  m_view.batteryStateTextView()->setText(text);
  updateBatteryState(Ion::Battery::voltage(), Ion::Battery::isCharging());
}

void BatteryTestController::updateBatteryState(float batteryLevel,
                                               bool batteryCharging) {
  constexpr int precision = Preferences::VeryLargeNumberOfSignificantDigits;
  constexpr int sizeForPrecision =
      PrintFloat::charSizeForFloatsWithPrecision(precision);
  constexpr size_t bufferLevelSize =
      ContentView::k_maxNumberOfCharacters + sizeForPrecision;
  char bufferLevel[bufferLevelSize];
  Poincare::Print::CustomPrintf(
      bufferLevel, bufferLevelSize, "Battery level: %*.*ef", batteryLevel,
      Preferences::PrintFloatMode::Decimal, precision);
  m_view.batteryLevelTextView()->setText(bufferLevel);

  constexpr size_t bufferChargingSize =
      ContentView::k_maxNumberOfCharacters + sizeForPrecision;
  char bufferCharging[bufferChargingSize];
  Poincare::Print::CustomPrintf(bufferCharging, bufferChargingSize,
                                "Battery charging: %s",
                                batteryCharging ? "yes" : "no");
  m_view.batteryChargingTextView()->setText(bufferCharging);
}

BatteryTestController::ContentView::ContentView()
    : SolidColorView(KDColorWhite),
      m_batteryStateView({.horizontalAlignment = KDGlyph::k_alignCenter}),
      m_batteryLevelView({.style = {.font = KDFont::Size::Small},
                          .horizontalAlignment = KDGlyph::k_alignCenter}),
      m_batteryChargingView({.style = {.font = KDFont::Size::Small},
                             .horizontalAlignment = KDGlyph::k_alignCenter}) {}

AbstractBufferTextView*
BatteryTestController::ContentView::batteryStateTextView() {
  return &m_batteryStateView;
}

AbstractBufferTextView*
BatteryTestController::ContentView::batteryLevelTextView() {
  return &m_batteryLevelView;
}

AbstractBufferTextView*
BatteryTestController::ContentView::batteryChargingTextView() {
  return &m_batteryChargingView;
}

void BatteryTestController::ContentView::setColor(KDColor color) {
  SolidColorView::setColor(color);
  m_batteryStateView.setBackgroundColor(color);
  m_batteryLevelView.setBackgroundColor(color);
  m_batteryChargingView.setBackgroundColor(color);
}

void BatteryTestController::ContentView::layoutSubviews(bool force) {
  setChildFrame(&m_batteryStateView,
                KDRect(0, 0, Ion::Display::Width, Ion::Display::Height / 2),
                force);
  KDSize textSize = KDFont::GlyphSize(KDFont::Size::Small);
  setChildFrame(&m_batteryLevelView,
                KDRect(0, Ion::Display::Height - 2 * textSize.height(),
                       Ion::Display::Width, textSize.height()),
                force);
  setChildFrame(&m_batteryChargingView,
                KDRect(0, Ion::Display::Height - textSize.height(),
                       Ion::Display::Width, textSize.height()),
                force);
}

int BatteryTestController::ContentView::numberOfSubviews() const { return 3; }

View* BatteryTestController::ContentView::subviewAtIndex(int index) {
  if (index == 0) {
    return &m_batteryStateView;
  }
  if (index == 1) {
    return &m_batteryLevelView;
  }
  return &m_batteryChargingView;
}

}  // namespace HardwareTest
