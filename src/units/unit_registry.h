#pragma once
#include "unit.h"

extern const UnitDef unit_osc;
extern const UnitDef unit_drum;
extern const UnitDef unit_sf2;
#ifndef __EMSCRIPTEN__
extern const UnitDef unit_clap;
#endif
extern const UnitDef unit_delay;
extern const UnitDef unit_dist;
extern const UnitDef unit_reverb;
extern const UnitDef unit_chorus;
extern const UnitDef unit_flanger;
extern const UnitDef unit_phaser;
extern const UnitDef unit_gran;

const UnitDef *unit_find(const char *id);
void unit_list(const UnitDef **out, int *count);
