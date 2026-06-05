#include "unit_registry.h"

#include <string.h>

static const UnitDef *REGISTRY[] = {
    &unit_osc,
    &unit_drum,
    &unit_sf2,
    &unit_clap,
    &unit_delay,
    &unit_dist,
    &unit_reverb,
    &unit_chorus,
    &unit_flanger,
    &unit_phaser,
    &unit_gran,
    NULL,
};

const UnitDef *unit_find(const char *id) {
  if (!id || !id[0])
    return NULL;
  for (int i = 0; REGISTRY[i]; i++)
    if (strcmp(REGISTRY[i]->id, id) == 0)
      return REGISTRY[i];
  return NULL;
}

void unit_list(const UnitDef **out, int *count) {
  *count = 0;
  for (int i = 0; REGISTRY[i]; i++) out[(*count)++] = REGISTRY[i];
}
