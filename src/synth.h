#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "tracker.h"

#define MAX_VOICES 64

typedef enum {
  ENV_ATTACK = 0,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE,
  ENV_OFF,
} EnvStage;

typedef struct {
  bool active;
  int channel;          // tracker channel that owns this voice
  uint8_t note;         // MIDI note
  float velocity;       // 0-1
  float freq;           // Hz
  float phase;          // oscillator phase 0-1
  float noise_val;      // last noise sample
  uint32_t noise_seed;  // noise LFSR state

  EnvStage env_stage;
  float env_level;  // current envelope level 0-1
  float env_time;   // time in current stage (seconds)

  SynthParams params;  // copy at note-on time
  float sample_rate;
} SynthVoice;

typedef struct {
  SynthVoice voices[MAX_VOICES];
  float sample_rate;
} Synth;

void synth_init(Synth *s, float sample_rate);
void synth_note_on(Synth *s, int channel, uint8_t note, uint8_t velocity, const SynthParams *params);
void synth_note_off(Synth *s, int channel, uint8_t note);
void synth_kill_channel(Synth *s, int channel);

// Render `frames` stereo float samples into out (interleaved L,R)
void synth_render(Synth *s, float *out, uint32_t frames);
