#pragma once
#include "imu_state.h"
#include "wheel_odom.h"

#include <optional>
#include <string_view>

namespace serial {

// Gear ratio and wheel geometry (motor -> wheel)
static constexpr float GEAR_RATIO    = 7.0f;
static constexpr float WHEEL_RADIUS_CM = 6.5f;  // cm

// Parse result variants
struct ImuFrame  { float yaw, pitch, roll; };   // degrees, as-is from STM32
struct RpmFrame  { float rpm; };

// Parse one line (without \r\n).
// Format:
//   @IMU:yaw,pitch,roll;;
//   @RPM:rpm;;
// Returns nullopt if line is unrecognised or malformed.
std::optional<ImuFrame> parse_imu(std::string_view line);
std::optional<RpmFrame> parse_rpm(std::string_view line);

// Convert RpmFrame to WheelOdom speed (cm/s).
// speed_cms = (rpm / GEAR_RATIO) * 2π * WHEEL_RADIUS_CM / 60
float rpm_to_speed_cms(float rpm);

} // namespace serial
