#include "synth.h"

#include <math.h>
#include <string.h>

#define TWO_PI 6.28318530717958647692f

static float note_to_freq(uint8_t note) {
  return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

static uint32_t lcg_next(uint32_t *state) {
  *state = *state * 1664525u + 1013904223u;
  return *state;
}

static float osc_sample(SynthVoice *v) {
  float p = v->phase;
  float s = 0.0f;
  switch (v->params.wave) {
    case WAVE_SINE:
      s = sinf(p * TWO_PI);
      break;
    case WAVE_SQUARE:
      s = (p < v->params.pw) ? 1.0f : -1.0f;
      break;
    case WAVE_SAW:
      s = 2.0f * p - 1.0f;
      break;
    case WAVE_TRIANGLE:
      s = (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
      break;
    case WAVE_NOISE:
      v->noise_val = (float)(int32_t)lcg_next(&v->noise_seed) / 2147483648.0f;
      s = v->noise_val;
      break;
    default:
      break;
  }
  float inc = (v->freq / v->sample_rate);
  v->phase += inc;
  if (v->phase >= 1.0f)
    v->phase -= 1.0f;
  return s;
}

static float env_process(SynthVoice *v, float dt) {
  SynthParams *p = &v->params;
  switch (v->env_stage) {
    case ENV_ATTACK:
      v->env_time += dt;
      if (p->attack <= 0.0001f) {
        v->env_level = 1.0f;
        v->env_stage = ENV_DECAY;
        v->env_time = 0.0f;
      } else {
        v->env_level = v->env_time / p->attack;
        if (v->env_level >= 1.0f) {
          v->env_level = 1.0f;
          v->env_stage = ENV_DECAY;
          v->env_time = 0.0f;
        }
      }
      break;
    case ENV_DECAY:
      v->env_time += dt;
      if (p->decay <= 0.0001f) {
        v->env_level = p->sustain;
        v->env_stage = ENV_SUSTAIN;
      } else {
        v->env_level = 1.0f - (1.0f - p->sustain) * (v->env_time / p->decay);
        if (v->env_time >= p->decay) {
          v->env_level = p->sustain;
          v->env_stage = ENV_SUSTAIN;
        }
      }
      break;
    case ENV_SUSTAIN:
      v->env_level = p->sustain;
      break;
    case ENV_RELEASE:
      v->env_time += dt;
      if (p->release <= 0.0001f) {
        v->env_level = 0.0f;
        v->env_stage = ENV_OFF;
        v->active = false;
      } else {
        v->env_level -= (p->sustain / p->release) * dt;
        if (v->env_level <= 0.0f) {
          v->env_level = 0.0f;
          v->env_stage = ENV_OFF;
          v->active = false;
        }
      }
      break;
    case ENV_OFF:
      v->active = false;
      return 0.0f;
  }
  return v->env_level;
}

void synth_init(Synth *s, float sample_rate) {
  memset(s, 0, sizeof(Synth));
  s->sample_rate = sample_rate;
}

void synth_note_on(Synth *s, int channel, uint8_t note, uint8_t velocity, const SynthParams *params) {
  // Find free voice (steal oldest active on same channel if full)
  SynthVoice *target = NULL;
  SynthVoice *oldest_same = NULL;

  for (int i = 0; i < MAX_VOICES; i++) {
    if (!s->voices[i].active) {
      target = &s->voices[i];
      break;
    }
    if (s->voices[i].channel == channel) {
      if (!oldest_same)
        oldest_same = &s->voices[i];
    }
  }

  if (!target) {
    target = oldest_same;
    if (!target)
      target = &s->voices[0];  // last resort steal
  }

  memset(target, 0, sizeof(SynthVoice));
  target->active = true;
  target->channel = channel;
  target->note = note;
  target->velocity = velocity / 127.0f;
  target->freq = note_to_freq(note);
  if (params->detune != 0.0f) {
    target->freq *= powf(2.0f, params->detune / 12.0f);
  }
  target->phase = 0.0f;
  target->noise_seed = 12345u + (uint32_t)note;
  target->env_stage = ENV_ATTACK;
  target->env_level = 0.0f;
  target->env_time = 0.0f;
  target->params = *params;
  target->sample_rate = s->sample_rate;
}

void synth_note_off(Synth *s, int channel, uint8_t note) {
  for (int i = 0; i < MAX_VOICES; i++) {
    SynthVoice *v = &s->voices[i];
    if (v->active && v->channel == channel && v->note == note && v->env_stage != ENV_RELEASE && v->env_stage != ENV_OFF) {
      v->env_stage = ENV_RELEASE;
      v->env_time = 0.0f;
    }
  }
}

void synth_kill_channel(Synth *s, int channel) {
  for (int i = 0; i < MAX_VOICES; i++) {
    if (s->voices[i].channel == channel) {
      s->voices[i].active = false;
    }
  }
}

void synth_render(Synth *s, float *out, uint32_t frames) {
  float dt = 1.0f / s->sample_rate;
  for (uint32_t f = 0; f < frames; f++) {
    float mixed = 0.0f;
    for (int i = 0; i < MAX_VOICES; i++) {
      SynthVoice *v = &s->voices[i];
      if (!v->active)
        continue;
      float env = env_process(v, dt);
      float samp = osc_sample(v);
      mixed += samp * env * v->velocity * v->params.volume;
    }
    // Soft clip to prevent distortion when many voices active
    mixed = tanhf(mixed * 0.7f);
    out[f * 2 + 0] = mixed;  // L
    out[f * 2 + 1] = mixed;  // R
  }
}
