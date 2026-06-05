#include <string.h>

#include "audio.h"
#include "input.h"
#include "raylib.h"
#include "tracker.h"
#include "ui.h"
#include "units/unit_registry.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static AudioEngine g_engine;
static UIState g_ui;
static AudioStream g_stream;

static void stream_callback(void *buf, unsigned int frames) {
  audio_fill_buffer(&g_engine, (float *)buf, frames);
}

static void main_loop(void) {
  input_update();
  ui_update(&g_ui);
  BeginDrawing();
  ui_draw(&g_ui);
  EndDrawing();
}

int main(void) {
  static TrackerSong song;

  tracker_init(&song);
  audio_init(&g_engine, &song);
  tracker_load(&song, "song.rpt");
  audio_set_save_dir(&g_engine, "song.rpt");

  ui_init(&g_ui, &song, &g_engine);

  InitWindow(WIN_W, WIN_H, "raypoketrack");
  SetTargetFPS(60);
  InitAudioDevice();

  g_stream = LoadAudioStream(AUDIO_SAMPLE_RATE, 32, 2);
  SetAudioStreamCallback(g_stream, stream_callback);
  PlayAudioStream(g_stream);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(main_loop, 0, 1);
#else
  while (!WindowShouldClose()) main_loop();

  audio_stop(&g_engine);
  StopAudioStream(g_stream);
  UnloadAudioStream(g_stream);
  CloseAudioDevice();
  audio_shutdown(&g_engine);
  CloseWindow();
#endif
  return 0;
}
