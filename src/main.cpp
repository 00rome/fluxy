// Entry point: F6 hotkey + (TODO) your existing buffer hooks.
//
// The original buffer logic (GJBaseGameLayer::handleButton, PlayerObject::update,
// ringJump, etc.) is NOT included here. Paste it into this file, and have it read
// the shared config from BufferConfig.hpp:
//     bufcfg::enabled()   -> skip buffering when false
//     bufcfg::allTicks()  -> tick limit for all inputs
//     bufcfg::waveTicks() -> tick limit for wave release
#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include "BufferConfig.hpp"

using namespace geode::prelude;

class $modify(BufKeyboard, CCKeyboardDispatcher) {
    // NOTE: if this doesn't compile, the signature differs in your SDK version
    // (newer versions add a `double timestamp` parameter). Match the SDK's header.
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (down && !repeat && key == KEY_F6 && PlayLayer::get()) {
            bufcfg::cycle();
            log::info("Input buffer: {}", bufcfg::summary());
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};
