#ifndef APPS_BATTERY_VIEW_H
#define APPS_BATTERY_VIEW_H

#include <escher/metric.h>
#include <escher/view.h>
#include <ion.h>

class BatteryView : public Escher::View {
 public:
  BatteryView()
      : m_chargeState(Ion::Battery::Charge::FULL),
        m_isCharging(false),
        m_isPlugged(false) {}
  bool setChargeState(Ion::Battery::Charge chargeState);
  bool setIsCharging(bool isCharging);
  bool setIsPlugged(bool isPlugged);
  void updateBatteryAnimation();
  void drawRect(KDContext* ctx, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override;
  constexpr static int k_flashHeight = 8;
  constexpr static int k_flashWidth = 4;
  constexpr static int k_tickHeight = 6;
  constexpr static int k_tickWidth = 8;

 private:
  void drawInsideBatteryLevel(KDContext* ctx, KDCoordinate width,
                              KDColor color = KDColorWhite) const;
  constexpr static KDCoordinate k_batteryHeight = 8;
  constexpr static KDCoordinate k_batteryWidth = 15;
  constexpr static KDCoordinate k_elementWidth = 1;
  constexpr static KDCoordinate k_capHeight = 4;
  constexpr static KDCoordinate k_separatorThickness =
      Escher::Metric::CellSeparatorThickness;
  constexpr static KDCoordinate k_batteryInsideX =
      k_elementWidth + k_separatorThickness;
  constexpr static KDCoordinate k_batteryInsideWidth =
      k_batteryWidth - 3 * k_elementWidth - 2 * k_separatorThickness;
  constexpr static uint8_t k_lowBatteryAnimationBlinks = 12;
  Ion::Battery::Charge m_chargeState;
  uint8_t m_lowBatteryAnimationState;
  bool m_isCharging;
  bool m_isPlugged;
};

#endif
