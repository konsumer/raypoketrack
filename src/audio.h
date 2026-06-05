#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "clap_host.h"
#include "tracker.h"
#include "units/unit_registry.h"

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BLOCK_SIZE 512

typedef struct {
  int song_row;
  int pattern_step;
} ChannelCursor;

typedef struct {
  TrackerSong *song;
  char save_dir[512];  // directory of loaded save file, for relative path resolution

  bool playing;
  bool pattern_loop;           // true = loop one pattern instead of full song
  uint8_t loop_pattern_idx;    // pattern index to loop (when pattern_loop)
  uint32_t loop_channel_mask;  // bitmask: channels that use loop_pattern_idx
  ChannelCursor cursors[SONG_CHANNELS];
  uint64_t tick_counter;
  uint32_t samples_per_tick;
  uint32_t sample_acc;
  uint8_t active_note[SONG_CHANNELS];
  uint8_t active_inst[SONG_CHANNELS];  // which instrument is loaded per channel

  // Per-channel per-slot unit states + which def created them
  UnitState *chan_states[SONG_CHANNELS][CHAIN_MAX];
  const UnitDef *chan_defs[SONG_CHANNELS][CHAIN_MAX];

  // Preview channel
  UnitState *preview_states[CHAIN_MAX];
  const UnitDef *preview_defs[CHAIN_MAX];
  uint8_t preview_inst;

  // Temp buffers for per-channel mixing
  float tmp_l[AUDIO_BLOCK_SIZE];
  float tmp_r[AUDIO_BLOCK_SIZE];
} AudioEngine;

void audio_init(AudioEngine *eng, TrackerSong *song);
void audio_shutdown(AudioEngine *eng);

// Set base directory for relative path resolution (call after load/save)
void audio_set_save_dir(AudioEngine *eng, const char *save_file_path);
void audio_play(AudioEngine *eng);
void audio_play_pattern(AudioEngine *eng, uint8_t pattern_idx);
void audio_stop(AudioEngine *eng);
bool audio_is_playing(const AudioEngine *eng);

// Preview a note through an instrument (for live editing feedback)
// Destroy live states for inst_idx so they rebuild cleanly after slot changes
void audio_rebuild_instrument(AudioEngine *eng, uint8_t inst_idx);

void audio_preview_note(AudioEngine *eng, uint8_t inst_idx, uint8_t note);
void audio_preview_kill(AudioEngine *eng);

void audio_fill_buffer(AudioEngine *eng, float *out, uint32_t frames);
