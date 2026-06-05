I would like to make something similar to LSDJ or m8 tracker (tracker driven by joystick with a/b/x/y/start/select/dpad) or https://4trk.net/ that uses raylib + raygui. I want it to follow some sort of standard, like using VST or ladpsa or something, so it's easy to add effects and sound-generators, on all platforms.

CLAP over LADSPA for new work — C ABI, no GUI required, supports instruments + effects, truly cross-platform. LADSPA MIDI is nonstandard anyway.

Here's proposed architecture before I start writing:

Data model (LSDJ/M8-style):

- Song → Chains[16] per channel
- Pattern → Steps[16] (note, velocity, instrument, FX params)
- Channel → instrument (CLAP plugin) + effects chain (CLAP plugins)

Screens (navigate sort of like like M8 or other trackers.)

Use SELECT+direction to choose screen.

- LEFT: SONG
- UP: PATTERN
- DOWN: INSTRUMENT/SAMPLE (which is effect-chain, in this case) - normally you can attach samples to any keys, so eventually we should figure this out
  with effects
- RIGHT: global things, for the mod

pressing A with arrow adjusts "current value" everywhere, so in SONG, it sets PATTERN, then you press select+up to edit that pattern
pressing B erases/cancels "current value" everywhere (delete pattern, note, reset instrument value, etc)

Files:

- src/main.c — init, main loop, screen dispatch
- src/tracker.h/c — song data model, sequencer engine
- src/audio.h/c — miniaudio callback, renders active patterns
- src/input.h/c — gamepad + keyboard → unified button events
- src/clap_host.h/c — load/run CLAP plugins as instruments/fx

CMake: FetchContent for clap headers (header-only, ~50KB).

Engine first, UI later. Let me scaffold the full engine:

- tracker.h/c — song data model (song/chain/phrase/step)
- input.h/c — gamepad + keyboard → unified buttons
- synth.h/c — built-in oscillator voices (test without CLAP)
- clap_host.h/c — CLAP plugin host (instruments + effects)
- audio.h/c — sequencer + miniaudio mixing
- main.c — minimal text display test harness