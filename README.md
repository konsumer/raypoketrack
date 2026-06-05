This is a cross-platform joystick-driven music-tracker.

It uses [CLAP](https://github.com/free-audio/clap) for sound-generation and effects, which you can sequence. This allows a wide variety of instrument-types, from the same interface.

## usage

You should be able to track quickly with a joystick, or keys:

![keys](./keys.svg)

| Input | Screen | Purpose |
|---|---|---|
| SELECT + ←/↑/↓/→ | Any | Switch to Song / Pattern / Instrument / Menu screen |
| START | Any | Play/stop |
| START | Pattern | Loop current pattern only (not full song) |
| ↑/↓ | Song | Move cursor row |
| ←/→ | Song | Scroll visible channels |
| A + ↑/↓ | Song | Set pattern number in focused cell |
| B | Song | Clear cell |
| L / R | Pattern | Previous / next pattern |
| ↑/↓ | Pattern | Move cursor row |
| ←/→ | Pattern | Move cursor column (note → vel → inst → fx…) |
| A + ↑/↓ | Pattern, note col | Step note up/down by current scale |
| A + ←/→ | Pattern, note col | Octave up/down |
| A + B | Pattern, note col | Insert NOTE-OFF |
| B | Pattern, note col | Clear note |
| A + ↑/↓ | Pattern, velocity col | Change velocity ±1 |
| A + ←/→ | Pattern, velocity col | Change velocity ±16 |
| B | Pattern, velocity col | Reset velocity to 100 |
| A + ↑/↓ | Pattern, instrument col | Change instrument number |
| B | Pattern, instrument col | Reset instrument to 0 |
| A + ↑/↓ | Pattern, FX col | Cycle FX type |
| A + ↑/↓ | Pattern, FX value col | Change FX value ±1 |
| A + ←/→ | Pattern, FX value col | Change FX value ±16 |
| B | Pattern, FX col | Clear FX |
| L / R | Instrument | Previous / next instrument |
| ↑/↓ | Instrument, slot panel | Navigate chain slots |
| → | Instrument, slot panel | Enter param panel for selected slot |
| A + ↑/↓ | Instrument, slot panel | Cycle unit type in slot |
| A + B | Instrument, slot panel | Toggle slot enabled/disabled |
| B | Instrument, slot panel | Clear slot |
| ↑/↓ | Instrument, param panel | Navigate params |
| ← | Instrument, param panel | Back to slot panel |
| A + ↑/↓ | Instrument, param panel | Change param value ±1 |
| A + ←/→ | Instrument, param panel | Change param value ±16 |
| B | Instrument, param panel | Reset param to default |
| A (Enter/Return) | Instrument, FILE row | Open file picker |
| B | Instrument, FILE row | Clear file path |
| ↑/↓ | Menu | Navigate items |
| A + ↑/↓ | Menu, BPM | Change BPM ±1 |
| A + ←/→ | Menu, BPM | Change BPM ±10 |
| A + ↑/↓ | Menu, KEY | Change scale root note |
| A + ↑/↓ | Menu, SCALE | Cycle scale (chromatic, major, minor, Persian…) |
| A + A | Menu, SAVE/LOAD/NEW | Confirm action |

It may seem a bit inscrutable at first, but input is meant to be consistent and fast with a joystick, so once you get the hang of it, it should work well. A is "change this value" and B is "delete/reset/cancel", SELECT+arrow is "change screen" and START is "play song/pattern".

## development

```sh
# build native
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# build web
emcmake cmake -B build-web -DCMAKE_BUILD_TYPE=Release -DPLATFORM=Web
cmake --build build-web --parallel

# run local watching web-server
npx -y live-server webroot --mount=/raypoketrack.mjs:./build-web/raypoketrack.mjs

# format code
find . -name "*.c" -o -name "*.h" | grep -v build | xargs clang-format -i
```