#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define UNIT_MAX_PARAMS 8
#define UNIT_ID_LEN 8

// Opaque per-instance runtime state
typedef struct UnitState UnitState;

typedef struct {
  const char *id;           // short id: "osc","drum","delay","dist"
  const char *name;         // display name
  const char *data_hint;    // placeholder shown when slot->data is empty (NULL = no data field)
  const char *file_filter;  // file dialog filter pattern e.g. "*.sf2" (NULL = any)
  bool is_source;           // true=generates audio, false=processes audio
  int num_params;
  const char *param_names[UNIT_MAX_PARAMS];
  uint8_t param_defaults[UNIT_MAX_PARAMS];

  UnitState *(*create)(float sample_rate);
  void (*destroy)(UnitState *s);

  // Called after create with slot's data string and save-file base directory.
  // Use for loading external files (SF2, CLAP) with relative-path support.
  // base_dir is the directory containing the save file (e.g. "/home/user/music/").
  // data is the raw string from ChainSlot.data (may be empty).
  void (*set_data)(UnitState *s, const char *data, const char *base_dir);

  void (*note_on)(UnitState *s, uint8_t note, uint8_t vel, const uint8_t *params);
  void (*note_off)(UnitState *s, uint8_t note);
  void (*kill)(UnitState *s);

  // Sources: in_l/in_r are NULL; ADD output to out_l/out_r (don't clear first).
  // Effects: read from in_l/in_r, write to out_l/out_r (may be same buffers).
  void (*render)(UnitState *s, const uint8_t *params,
                 const float *in_l, const float *in_r,
                 float *out_l, float *out_r, uint32_t frames);
} UnitDef;

// Resolve a path relative to base_dir; writes result to out.
// If path is already absolute, copies as-is.
static inline void unit_resolve_path(const char *base_dir, const char *path,
                                     char *out, int sz) {
  if (!path || !path[0]) {
    out[0] = '\0';
    return;
  }
#ifdef _WIN32
  int abs = (path[0] == '/' || path[0] == '\\' || (path[1] == ':'));
#else
  int abs = (path[0] == '/');
#endif
  if (abs || !base_dir || !base_dir[0]) {
    snprintf(out, sz, "%s", path);
  } else {
    snprintf(out, sz, "%s%s", base_dir, path);
  }
}

// Map 00-FF param to float range [lo, hi]
static inline float p2f(uint8_t p, float lo, float hi) {
  return lo + (p / 255.0f) * (hi - lo);
}
