#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>

// Shared config between the buffer logic and the UI.
// In your existing hooks, replace the hardcoded stage values / F6 cycle with these.
namespace bufcfg {

struct Stage {
    int allTicks;   // ticks buffered for all inputs
    int waveTicks;  // ticks buffered for wave release
    const char* label;
};

// index 4 = OFF (100% raw input), matches the F6 cycle in v4.7
inline constexpr Stage STAGES[5] = {
    {2, 2,  "Stage 1"},
    {4, 4,  "Stage 2"},
    {8, 8,  "Stage 3"},
    {16, 8, "Stage 4"},
    {0, 0,  "OFF"},
};

inline int stage() {
    return std::clamp(geode::Mod::get()->getSavedValue<int>("stage", 0), 0, 4);
}
inline void setStage(int s) {
    geode::Mod::get()->setSavedValue<int>("stage", std::clamp(s, 0, 4));
}
inline void cycle() { setStage((stage() + 1) % 5); }          // use this for F6
inline bool enabled()  { return stage() != 4; }
inline int  allTicks() { return STAGES[stage()].allTicks; }
inline int  waveTicks(){ return STAGES[stage()].waveTicks; }

inline bool hudEnabled() { return geode::Mod::get()->getSavedValue<bool>("hud", true); }
inline void setHud(bool v) { geode::Mod::get()->setSavedValue<bool>("hud", v); }

inline std::string summary() {
    if (!enabled()) return "OFF (raw input)";
    return fmt::format("{} All / {} Wave", allTicks(), waveTicks());
}
inline std::string frames(int ticks) {
    return fmt::format("{:.2f}f @ 60fps", ticks / 4.f);  // 240 TPS -> 60 fps
}

} // namespace bufcfg
