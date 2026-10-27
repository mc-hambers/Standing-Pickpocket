#pragma once
#include <cstdint>
// Obtain after SKSE kPostLoad using GetModuleHandleW + GetProcAddress.
// API functions use the C calling convention. Tokens must remain stable until release.
namespace StandingPickpocketAPI {
struct V1 {
    std::uint32_t version;
    bool (*SetDistanceOverride)(const void* ownerToken,float multiplier);
    bool (*ReleaseDistanceOverride)(const void* ownerToken);
    float (*GetEffectiveDistanceMultiplier)();
    bool (*IsDistanceOverridden)();
};
using Request = const V1* (*)(std::uint32_t version);
// Export: StandingPickpocket_RequestAPI. Request version 1; unsupported versions return null.
}
