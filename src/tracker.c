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
  // fx fields default to TRACKER_EMPTY (no param automation)
  for (int pi = 0; pi < NUM_PATTERNS; pi++)
    for (int si = 0; si < PATTERN_STEPS; si++)
      for (int fi = 0; fi < FX_PER_STEP; fi++)
        song->pattern_data[pi].steps[si].fx[fi] = TRACKER_EMPTY;
  for (int i = 0; i < NUM_INSTRUMENTS; i++)
    snprintf(song->instruments[i].name, 16, "INST%02X", i);
  song->bpm = 120;
  song->swing = 0;
  song->loop = true;
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

// Attachment section appended after TrackerSong struct:
//   [4] magic 0x32545052 ('RPT2')
//   [4] count
//   For each:  [4] path_len, [path_len] path, [4] data_len, [data_len] data

#define RPT2_MAGIC 0x32545052u

#include <sys/stat.h>
#include <stdlib.h>

static void save_dir_of(const char *file_path, char *dir, int sz) {
    strncpy(dir, file_path, sz - 1);
    char *sep = strrchr(dir, '/');
    if (!sep) sep = strrchr(dir, '\\');
    if (sep) *(sep + 1) = '\0';
    else { dir[0] = '.'; dir[1] = '/'; dir[2] = '\0'; }
}

static const char *basename_of(const char *p) {
    const char *s = strrchr(p, '/');
    if (!s) s = strrchr(p, '\\');
    return s ? s + 1 : p;
}

static void ensure_parent_dir(const char *file_path) {
    char dir[512];
    save_dir_of(file_path, dir, sizeof(dir));
    if (dir[0] && !(dir[0] == '.' && dir[1] == '/'))
#ifdef _WIN32
        mkdir(dir);
#else
        mkdir(dir, 0755);
#endif
}

bool tracker_save(const TrackerSong *song, const char *path) {
    char save_dir[512];
    save_dir_of(path, save_dir, sizeof(save_dir));

    // Collect file attachments from all chain slot data fields
    typedef struct { char key[512]; unsigned char *data; uint32_t size; } Att;
    Att  *atts = NULL;
    int   natt = 0;

    for (int i = 0; i < NUM_INSTRUMENTS; i++) {
        for (int s = 0; s < CHAIN_MAX; s++) {
            const char *d = song->instruments[i].chain[s].data;
            if (!d[0]) continue;
            // Handle CLAP tab-separated "path\tplugin_id"
            char fpath[512];
            strncpy(fpath, d, sizeof(fpath) - 1);
            char *tab = strchr(fpath, '\t');
            if (tab) *tab = '\0';
            // Resolve relative paths
            char resolved[512];
            if (fpath[0] == '/' || fpath[0] == '\\' || fpath[1] == ':')
                strncpy(resolved, fpath, sizeof(resolved) - 1);
            else
                snprintf(resolved, sizeof(resolved), "%s%s", save_dir, fpath);
            // Skip if already added
            bool dup = false;
            for (int k = 0; k < natt; k++)
                if (strcmp(atts[k].key, resolved) == 0) { dup = true; break; }
            if (dup) continue;
            // Try to load the file (skip if not found or too large)
            int fsize = 0;
            unsigned char *fdata = LoadFileData(resolved, &fsize);
            if (!fdata || fsize <= 0 || fsize > 64 * 1024 * 1024) {
                if (fdata) UnloadFileData(fdata);
                continue;
            }
            atts = realloc(atts, (natt + 1) * sizeof(Att));
            strncpy(atts[natt].key, resolved, sizeof(atts[natt].key) - 1);
            atts[natt].data = fdata;
            atts[natt].size = (uint32_t)fsize;
            natt++;
        }
    }

    // Build complete save buffer
    size_t total = sizeof(TrackerSong);
    if (natt > 0) {
        total += 8; // magic + count
        for (int k = 0; k < natt; k++)
            total += 4 + strlen(atts[k].key) + 1 + 4 + atts[k].size;
    }

    unsigned char *buf = malloc(total);
    if (!buf) {
        for (int k = 0; k < natt; k++) UnloadFileData(atts[k].data);
        free(atts);
        return false;
    }

    memcpy(buf, song, sizeof(TrackerSong));

    if (natt > 0) {
        unsigned char *p = buf + sizeof(TrackerSong);
        uint32_t magic = RPT2_MAGIC, cnt = (uint32_t)natt;
        memcpy(p, &magic, 4); p += 4;
        memcpy(p, &cnt,   4); p += 4;
        for (int k = 0; k < natt; k++) {
            uint32_t plen = (uint32_t)(strlen(atts[k].key) + 1);
            memcpy(p, &plen, 4);              p += 4;
            memcpy(p, atts[k].key, plen);     p += plen;
            memcpy(p, &atts[k].size, 4);      p += 4;
            memcpy(p, atts[k].data, atts[k].size); p += atts[k].size;
            UnloadFileData(atts[k].data);
        }
    }
    free(atts);

    bool ok = SaveFileData(path, buf, (int)total);
    free(buf);
    return ok;
}

bool tracker_load(TrackerSong *song, const char *path) {
    int size = 0;
    unsigned char *data = LoadFileData(path, &size);
    if (!data) return false;

    bool ok = ((size_t)size >= sizeof(TrackerSong));
    if (ok) memcpy(song, data, sizeof(TrackerSong));

    // Read attachment section if present
    if (ok && (size_t)size > sizeof(TrackerSong) + 8) {
        unsigned char *p = data + sizeof(TrackerSong);
        unsigned char *end = data + size;
        uint32_t magic;
        memcpy(&magic, p, 4);
        if (magic == RPT2_MAGIC) {
            p += 4;
            uint32_t cnt; memcpy(&cnt, p, 4); p += 4;
            char save_dir[512];
            save_dir_of(path, save_dir, sizeof(save_dir));
            for (uint32_t k = 0; k < cnt && p + 8 <= end; k++) {
                uint32_t plen; memcpy(&plen, p, 4); p += 4;
                if (p + plen + 4 > end) break;
                char att_path[512];
                strncpy(att_path, (char *)p, sizeof(att_path) - 1);
                p += plen;
                uint32_t dlen; memcpy(&dlen, p, 4); p += 4;
                if (p + dlen > end) break;
                // Write file to its stored path; fallback to save_dir/basename
                ensure_parent_dir(att_path);
                if (!SaveFileData(att_path, p, (int)dlen)) {
                    char fallback[512];
                    snprintf(fallback, sizeof(fallback), "%s%s",
                             save_dir, basename_of(att_path));
                    if (SaveFileData(fallback, p, (int)dlen)) {
                        // Update matching slot->data paths
                        for (int i = 0; i < NUM_INSTRUMENTS; i++)
                            for (int s = 0; s < CHAIN_MAX; s++) {
                                char *dd = song->instruments[i].chain[s].data;
                                if (strcmp(dd, att_path) == 0)
                                    strncpy(dd, fallback, 238);
                            }
                    }
                }
                p += dlen;
            }
        }
    }

    UnloadFileData(data);
    return ok;
}
