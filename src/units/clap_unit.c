// CLAP plugin wrapper unit
// data field format: "relative/path/to/plugin.clap\tcom.vendor.plugin-id"
// If plugin_id part is omitted, loads first plugin in the bundle.
// P0-P7: reserved (future CLAP param mapping)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../clap_host.h"
#include "unit.h"

struct UnitState {
  ClapPlugin *plugin;
  float sample_rate;
  bool is_source;
  char resolved_path[512];
  char plugin_id[128];
};

static UnitState *clap_unit_create(float sr) {
  UnitState *s = calloc(1, sizeof(*s));
  s->sample_rate = sr;
  return s;
}

static void clap_unit_destroy(UnitState *s) {
  if (s->plugin)
    clap_host_unload(s->plugin);
  free(s);
}

static void clap_unit_set_data(UnitState *s, const char *data, const char *base_dir) {
  if (!data || !data[0])
    return;

  // Parse: "path\tplugin_id"
  char buf[640];
  strncpy(buf, data, sizeof(buf) - 1);
  char *tab = strchr(buf, '\t');
  const char *id = "";
  if (tab) {
    *tab = '\0';
    id = tab + 1;
  }

  unit_resolve_path(base_dir, buf, s->resolved_path, sizeof(s->resolved_path));
  strncpy(s->plugin_id, id, sizeof(s->plugin_id) - 1);

  // Unload previous
  if (s->plugin) {
    clap_host_unload(s->plugin);
    s->plugin = NULL;
  }

  s->plugin = clap_host_load(s->resolved_path, s->plugin_id[0] ? s->plugin_id : NULL,
                             s->sample_rate, 512);
  if (s->plugin)
    s->is_source = clap_host_is_instrument(s->plugin);
}

static void clap_unit_note_on(UnitState *s, uint8_t note, uint8_t vel, const uint8_t *p) {
  (void)p;
  if (s->plugin && s->is_source)
    clap_host_note_on(s->plugin, note, vel, 0);
}
static void clap_unit_note_off(UnitState *s, uint8_t note) {
  if (s->plugin && s->is_source)
    clap_host_note_off(s->plugin, note, 0);
}
static void clap_unit_kill(UnitState *s) {
  if (s->plugin) {
    // Send note-off for all notes
    for (int n = 0; n < 128; n++) clap_host_note_off(s->plugin, (uint8_t)n, 0);
  }
}

static void clap_unit_render(UnitState *s, const uint8_t *p,
                             const float *in_l, const float *in_r,
                             float *out_l, float *out_r, uint32_t frames) {
  (void)p;
  if (!s->plugin)
    return;
  float tmp_l[512] = {0}, tmp_r[512] = {0};
  if (frames > 512)
    frames = 512;
  clap_host_process(s->plugin,
                    s->is_source ? NULL : in_l,
                    s->is_source ? NULL : in_r,
                    tmp_l, tmp_r, frames);
  for (uint32_t f = 0; f < frames; f++) {
    out_l[f] += tmp_l[f];
    out_r[f] += tmp_r[f];
  }
}

const UnitDef unit_clap = {
    .id = "clap",
    .name = "CLAP",
    .data_hint = "plugin.clap",
    .file_filter = "*.clap",
    .is_source = true,  // actual role set in set_data
    .num_params = 0,
    .param_names = {},
    .param_defaults = {},
    .create = clap_unit_create,
    .destroy = clap_unit_destroy,
    .set_data = clap_unit_set_data,
    .note_on = clap_unit_note_on,
    .note_off = clap_unit_note_off,
    .kill = clap_unit_kill,
    .render = clap_unit_render,
};
