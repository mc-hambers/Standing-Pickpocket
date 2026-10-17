#pragma once
constexpr bool BlockVanillaPickpocket(bool manuallyDisabled, bool inRange) noexcept {
    return manuallyDisabled || !inRange;
}
