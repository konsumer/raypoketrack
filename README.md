This is a cross-platform joystick-driven music-tracker.

It uses [CLAP](https://github.com/free-audio/clap) for sound-generation and effects, which you can sequence. This allows a wide variety of instrument-types, from the same interface.

## usage

You should be able to track quickly with a joystick, or keys:

![keys](./keys.png)

```
┌───────────────────────┬────────────────┬─────────────────────────┐
│   In phrase screen    │      Keys      │         Action          │
├───────────────────────┼────────────────┼─────────────────────────┤
│ Navigate              │ Arrows         │ Move cursor row/col     │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + Up/Down           │ X + Arrows     │ Change note semitone    │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + Left/Right        │ X + ←/→        │ Octave down/up          │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + B                 │ X + Z          │ Place NOTE_OFF          │
├───────────────────────┼────────────────┼─────────────────────────┤
│ SELECT + A + B        │ RShift + X + Z │ Clear note (empty)      │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + Up/Down on VEL    │ X + ↑↓         │ Change velocity ±1      │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + Left/Right on VEL │ X + ←/→        │ Change velocity ±16     │
├───────────────────────┼────────────────┼─────────────────────────┤
│ A + Up/Down on INS    │ X + ↑↓         │ Change instrument       │
├───────────────────────┼────────────────┼─────────────────────────┤
│ SELECT + A on INS col │ RShift + X     │ Enter instrument editor │
├───────────────────────┼────────────────┼─────────────────────────┤
│ B                     │ Z              │ Back to chain           │
└───────────────────────┴────────────────┴─────────────────────────┘
```

It may seem a bit inscruitable at first, but input is meant to be consistant, and fast with a joystuck, so once you get the hang of it, it should work well.

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