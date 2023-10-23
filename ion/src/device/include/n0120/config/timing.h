#ifndef ION_DEVICE_N0120_SHARED_DRIVERS_CONFIG_TIMING_H
#define ION_DEVICE_N0120_SHARED_DRIVERS_CONFIG_TIMING_H

namespace Ion {
namespace Device {
namespace Timing {
namespace Config {

/* When sleeping, we set the D1 prescaler to cut the frequency 512. */
#warning experimental values
#if DEBUG
constexpr static int LoopsPerMillisecondLowFrequency = 35;
#else
constexpr static int LoopsPerMillisecondLowFrequency = 146;
#endif
constexpr static int LoopsPerMicrosecond = 86;

}  // namespace Config
}  // namespace Timing
}  // namespace Device
}  // namespace Ion

#endif
