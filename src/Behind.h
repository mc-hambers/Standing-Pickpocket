#pragma once
#include <cmath>
inline bool IsBehind(double dx, double dy, double yaw) noexcept {
    const double length=std::hypot(dx,dy);
    return length>0.001 && (dx*std::sin(yaw)+dy*std::cos(yaw))/length <= -0.5;
}
