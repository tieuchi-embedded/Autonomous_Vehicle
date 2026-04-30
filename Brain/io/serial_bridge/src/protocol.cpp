#include "serial/protocol.hpp"

#include <cstdio>
#include <cmath>

namespace serial {

std::optional<ImuFrame> parse_imu(std::string_view line) {
    // Expected: @IMU:yaw,pitch,roll;;
    if (line.size() < 6 || line.substr(0, 5) != "@IMU:") return std::nullopt;

    ImuFrame f{};
    // strip trailing ;;
    auto body = line.substr(5);
    if (body.size() >= 2 && body.substr(body.size() - 2) == ";;")
        body = body.substr(0, body.size() - 2);

    if (std::sscanf(body.data(), "%f,%f,%f", &f.yaw, &f.pitch, &f.roll) != 3)
        return std::nullopt;
    return f;
}

std::optional<RpmFrame> parse_rpm(std::string_view line) {
    // Expected: @RPM:value;;
    if (line.size() < 6 || line.substr(0, 5) != "@RPM:") return std::nullopt;

    RpmFrame f{};
    auto body = line.substr(5);
    if (body.size() >= 2 && body.substr(body.size() - 2) == ";;")
        body = body.substr(0, body.size() - 2);

    if (std::sscanf(body.data(), "%f", &f.rpm) != 1) return std::nullopt;
    return f;
}

float rpm_to_speed_cms(float rpm) {
    // wheel_rpm = motor_rpm / GEAR_RATIO
    // speed = wheel_rpm * 2π * r / 60
    return (rpm / GEAR_RATIO) * (2.0f * static_cast<float>(M_PI)) * WHEEL_RADIUS_CM / 60.0f;
}

} // namespace serial
