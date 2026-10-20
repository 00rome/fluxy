# Flux

Geode mod for Geometry Dash: a trajectory-based 1-2 frame press and wave release input buffer,
with a customizable in-game menu (cyan lightning-bolt theme by default).

## Menu
Pause screen -> **Flux** button.

**Buffer tab**: Stage 1-4 / OFF, live tick and frame readout, HUD label toggle. F6 still cycles stages in-level.

**Appearance tab**: accent color (R/G/B), opacity, width, height, corner roundness, scale,
position (left/center/right), reset. Everything is saved between sessions.

## Building
1. Install the [Geode SDK](https://docs.geode-sdk.org/getting-started/) and set `GEODE_SDK`.
2. `cmake -B build` then `cmake --build build --config RelWithDebInfo`
3. The `.geode` file appears in `build/`.

## Layout
- `src/BufferConfig.hpp`: stage config shared by the buffer logic and the menu
- `src/FluxTheme.hpp`: saved theme values
- `src/FluxMenu.cpp`: menu, pause button, HUD label
- `src/main.cpp`: F6 hotkey, plus where the buffer hooks go
