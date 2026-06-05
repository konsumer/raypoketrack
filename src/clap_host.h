#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "tracker.h"

// Opaque CLAP plugin instance
typedef struct ClapPlugin ClapPlugin;

// MIDI event for CLAP processing
typedef struct {
  uint8_t type;  // 0=note_on, 1=note_off, 2=param_change
  uint8_t note;
  uint8_t velocity;
  uint8_t channel;
  uint32_t offset;  // sample offset within block
} ClapMidiEvent;

#define MAX_CLAP_EVENTS 64

ClapPlugin *clap_host_load(const char *path, const char *plugin_id, float sample_rate, uint32_t block_size);
void clap_host_unload(ClapPlugin *p);

void clap_host_note_on(ClapPlugin *p, uint8_t note, uint8_t velocity, uint32_t offset);
void clap_host_note_off(ClapPlugin *p, uint8_t note, uint32_t offset);

// Process one block; in_l/in_r may be NULL for instruments
// out_l/out_r are output buffers of `frames` floats
void clap_host_process(ClapPlugin *p,
                       const float *in_l, const float *in_r,
                       float *out_l, float *out_r,
                       uint32_t frames);

bool clap_host_is_instrument(ClapPlugin *p);
const char *clap_host_name(ClapPlugin *p);
