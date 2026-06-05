#include "tracker.h"

#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "units/unit_registry.h"

const ScaleDef SCALES[NUM_SCALES] = {
    // --- Chromatic ---
    {"CHROM", 12, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}},
    // --- Western diatonic modes ---
    {"MAJOR", 7, {0, 2, 4, 5, 7, 9, 11}},
    {"MINOR", 7, {0, 2, 3, 5, 7, 8, 10}},
    {"DORIAN", 7, {0, 2, 3, 5, 7, 9, 10}},
    {"PHRYG", 7, {0, 1, 3, 5, 7, 8, 10}},
    {"LYDIAN", 7, {0, 2, 4, 6, 7, 9, 11}},
    {"MIXOLY", 7, {0, 2, 4, 5, 7, 9, 10}},
    {"LOCRIAN", 7, {0, 1, 3, 5, 6, 8, 10}},
    // --- Harmonic / melodic variants ---
    {"HM.MIN", 7, {0, 2, 3, 5, 7, 8, 11}},
    {"ML.MIN", 7, {0, 2, 3, 5, 7, 9, 11}},
    {"HM.MAJ", 7, {0, 2, 4, 5, 7, 8, 11}},
    // --- Jazz / altered modes ---
    {"LYD.DOM", 7, {0, 2, 4, 6, 7, 9, 10}},
    {"PHR.DOM", 7, {0, 1, 4, 5, 7, 8, 10}},
    {"LYD.AUG", 7, {0, 2, 4, 6, 8, 9, 11}},
    {"ALTERED", 7, {0, 1, 3, 4, 6, 8, 10}},
    {"UK.DORI", 7, {0, 2, 3, 6, 7, 9, 10}},
    // --- Pentatonic / blues ---
    {"PENT.MJ", 5, {0, 2, 4, 7, 9}},
    {"PENT.MN", 5, {0, 3, 5, 7, 10}},
    {"BLUES", 6, {0, 3, 5, 6, 7, 10}},
    {"MJ.BLUE", 6, {0, 2, 3, 4, 7, 9}},
    {"EGYPT", 5, {0, 2, 5, 7, 10}},
    // --- Symmetric ---
    {"W.TONE", 6, {0, 2, 4, 6, 8, 10}},
    {"DIM.H-W", 8, {0, 1, 3, 4, 6, 7, 9, 10}},
    {"DIM.W-H", 8, {0, 2, 3, 5, 6, 8, 9, 11}},
    {"AUGMENT", 6, {0, 3, 4, 7, 8, 11}},
    {"TRITONE", 6, {0, 1, 4, 6, 7, 10}},
    {"PROMETH", 6, {0, 2, 4, 6, 9, 10}},
    // --- Middle Eastern ---
    {"PERSIAN", 7, {0, 1, 4, 5, 6, 8, 11}},
    {"BYZANT", 7, {0, 1, 4, 5, 7, 8, 11}},
    {"ENIGMA", 7, {0, 1, 4, 6, 8, 10, 11}},
    // --- Japanese ---
    {"HIRAJSH", 5, {0, 2, 3, 7, 8}},
    {"IN", 5, {0, 1, 5, 7, 8}},
    {"INSEN", 5, {0, 1, 5, 7, 10}},
    {"IWATO", 5, {0, 1, 5, 6, 10}},
    {"YO", 5, {0, 2, 5, 7, 9}},
    {"MAN.GON", 5, {0, 3, 5, 8, 10}},
    // --- Hungarian ---
    {"HUNG.MN", 7, {0, 2, 3, 6, 7, 8, 11}},
    {"HUNG.MJ", 7, {0, 3, 4, 6, 7, 9, 10}},
    // --- Neapolitan ---
    {"NAPL.MJ", 7, {0, 1, 3, 5, 7, 9, 11}},
    {"NAPL.MN", 7, {0, 1, 3, 5, 7, 8, 11}},
    // --- Indian ragas ---
    {"TODI", 7, {0, 1, 3, 6, 7, 8, 11}},
    {"MARWA", 7, {0, 1, 4, 6, 7, 9, 11}},
    {"PURVI", 7, {0, 1, 4, 6, 7, 8, 11}},
    // --- 8-tone / bebop ---
    {"SPANISH", 8, {0, 1, 3, 4, 5, 6, 8, 10}},
    {"BEBOP.D", 8, {0, 2, 4, 5, 7, 9, 10, 11}},
};

const char *const SCALE_ROOT_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

uint8_t scale_next_note(uint8_t note, int dir, uint8_t scale_idx, uint8_t root) {
  if (scale_idx == 0) {
    // Chromatic: straight semitone step
    int n = (int)note + dir;
    if (n < 1)
      return 1;
    if (n > 127)
      return 127;
    return (uint8_t)n;
  }
  const ScaleDef *sc = &SCALES[scale_idx];
  int n = (int)note;
  for (int i = 0; i < 128; i++) {
    n += dir;
    if (n < 1)
      return 1;
    if (n > 127)
      return 127;
    int pc = ((n - (int)root) % 12 + 12) % 12;
    for (int j = 0; j < sc->len; j++)
      if (sc->intervals[j] == pc)
        return (uint8_t)n;
  }
  return note;
}

void tracker_inst_set_slot(TrackerInstrument *inst, int slot, const char *unit_id) {
  if (slot < 0 || slot >= CHAIN_MAX)
    return;
  ChainSlot *s = &inst->chain[slot];
  strncpy(s->unit_id, unit_id, UNIT_ID_LEN - 1);
  s->unit_id[UNIT_ID_LEN - 1] = '\0';
  s->enabled = true;
  const UnitDef *def = unit_find(unit_id);
  if (def) {
    for (int i = 0; i < UNIT_MAX_PARAMS; i++)
      s->params[i] = def->param_defaults[i];
  }
}

void tracker_init(TrackerSong *song) {
  memset(song, 0, sizeof(TrackerSong));
  memset(song->patterns, TRACKER_EMPTY, sizeof(song->patterns));
  for (int i = 0; i < NUM_INSTRUMENTS; i++)
    snprintf(song->instruments[i].name, 16, "INST%02X", i);
  song->bpm = 120;
  song->swing = 0;
  strncpy(song->name, "UNTITLED", 32);
}

void tracker_clear(TrackerSong *song) {
  uint8_t bpm = song->bpm;
  char name[32];
  strncpy(name, song->name, 32);
  tracker_init(song);
  song->bpm = bpm;
  strncpy(song->name, name, 32);
}

bool tracker_save(const TrackerSong *song, const char *path) {
  return SaveFileData(path, (void *)song, (int)sizeof(TrackerSong));
}

bool tracker_load(TrackerSong *song, const char *path) {
  int size = 0;
  unsigned char *data = LoadFileData(path, &size);
  if (!data)
    return false;
  bool ok = ((size_t)size == sizeof(TrackerSong));
  if (ok)
    memcpy(song, data, sizeof(TrackerSong));
  UnloadFileData(data);
  return ok;
}
