#pragma once
#include <Geode/Geode.hpp>
#include <algorithm>

// Flux UI theme: every field persists between sessions.
namespace flux {

struct Theme {
    int accentR = 0, accentG = 229, accentB = 255;   // cyan
    int bgR = 18, bgG = 16, bgB = 30;                // dark indigo
    float opacity = 0.88f;   // 0.30 - 1.00
    float width   = 360.f;   // 320 - 520
    float height  = 250.f;   // 230 - 380
    float radius  = 14.f;    // 0 - 24
    float scale   = 1.0f;    // 0.70 - 1.30
    int   pos     = 1;       // 0 left, 1 center, 2 right
};

inline Theme& theme() {
    static Theme t;
    return t;
}

inline void load() {
    auto m = geode::Mod::get();
    auto& t = theme();
    Theme d;
    t.accentR = m->getSavedValue<int>("flux-ar", d.accentR);
    t.accentG = m->getSavedValue<int>("flux-ag", d.accentG);
    t.accentB = m->getSavedValue<int>("flux-ab", d.accentB);
    t.opacity = m->getSavedValue<float>("flux-opacity", d.opacity);
    t.width   = m->getSavedValue<float>("flux-width", d.width);
    t.height  = m->getSavedValue<float>("flux-height", d.height);
    t.radius  = m->getSavedValue<float>("flux-radius", d.radius);
    t.scale   = m->getSavedValue<float>("flux-scale", d.scale);
    t.pos     = m->getSavedValue<int>("flux-pos", d.pos);
}

inline void save() {
    auto m = geode::Mod::get();
    auto& t = theme();
    m->setSavedValue<int>("flux-ar", t.accentR);
    m->setSavedValue<int>("flux-ag", t.accentG);
    m->setSavedValue<int>("flux-ab", t.accentB);
    m->setSavedValue<float>("flux-opacity", t.opacity);
    m->setSavedValue<float>("flux-width", t.width);
    m->setSavedValue<float>("flux-height", t.height);
    m->setSavedValue<float>("flux-radius", t.radius);
    m->setSavedValue<float>("flux-scale", t.scale);
    m->setSavedValue<int>("flux-pos", t.pos);
}

inline void reset() { theme() = Theme{}; save(); }

inline cocos2d::ccColor3B accent3() {
    auto& t = theme();
    return {(GLubyte)t.accentR, (GLubyte)t.accentG, (GLubyte)t.accentB};
}
inline cocos2d::ccColor4F accent4(float a = 1.f) {
    auto& t = theme();
    return {t.accentR / 255.f, t.accentG / 255.f, t.accentB / 255.f, a};
}
inline cocos2d::ccColor4F bg4(float a) {
    auto& t = theme();
    return {t.bgR / 255.f, t.bgG / 255.f, t.bgB / 255.f, a};
}

inline float lerp(float lo, float hi, float v) { return lo + (hi - lo) * std::clamp(v, 0.f, 1.f); }
inline float unlerp(float lo, float hi, float x) { return std::clamp((x - lo) / (hi - lo), 0.f, 1.f); }

} // namespace flux
