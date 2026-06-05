#include "audio.h"
#include "ui.h"
#include "units/unit_registry.h"
#ifndef __EMSCRIPTEN__
#include "tinyfiledialogs.h"
#endif
#include <stdio.h>
#include <string.h>

#define PANEL_W (WIN_W / 2)
#define INST_HEADER_Y (STATUS_H + 1)
#define INST_CONTENT_Y (INST_HEADER_Y + CH_H + 1)

// inst_row 0-7 = slot list, 8+ = param/data editing
// DATA row index = CHAIN_MAX + def->num_params  (one past last param)

static const UnitDef *slot_def(TrackerInstrument *inst, int s) {
  if (!inst->chain[s].unit_id[0])
    return NULL;
  return unit_find(inst->chain[s].unit_id);
}

void screen_instrument_update(UIState *ui) {
  TrackerInstrument *inst = &ui->song->instruments[ui->ctx_instrument];

  // (inst_data_editing kept for text-fallback path but file dialog is primary)

  bool edit = input_held(BTN_A);
  bool in_params = (ui->inst_row >= CHAIN_MAX);

  // L/R shoulder: cycle instrument (not while editing)
  if (!edit && !in_params) {
    if (ui_repeat(BTN_R) && ui->ctx_instrument < NUM_INSTRUMENTS - 1) {
      ui->ctx_instrument++;
      ui->inst_row = 0;
      inst = &ui->song->instruments[ui->ctx_instrument];
    }
    if (ui_repeat(BTN_L) && ui->ctx_instrument > 0) {
      ui->ctx_instrument--;
      ui->inst_row = 0;
      inst = &ui->song->instruments[ui->ctx_instrument];
    }
  }

  if (!in_params) {
    int slot = ui->inst_row;
    if (!edit) {
      if (ui_repeat(BTN_UP) && slot > 0)
        ui->inst_row--;
      if (ui_repeat(BTN_DOWN) && slot < CHAIN_MAX - 1)
        ui->inst_row++;
      if (ui_repeat(BTN_RIGHT) && slot_def(inst, slot))
        ui->inst_row = CHAIN_MAX;
      if (input_pressed(BTN_B)) {
        audio_rebuild_instrument(ui->engine, (uint8_t)ui->ctx_instrument);
        memset(&inst->chain[slot], 0, sizeof(ChainSlot));
      }
    } else {
      const UnitDef *defs[16];
      int nf = 0;
      unit_list(defs, &nf);
      ChainSlot *sl = &inst->chain[slot];

      if (ui_repeat(BTN_UP) || ui_repeat(BTN_DOWN)) {
        int cur_idx = -1;
        for (int i = 0; i < nf; i++)
          if (sl->unit_id[0] && strcmp(defs[i]->id, sl->unit_id) == 0) {
            cur_idx = i;
            break;
          }
        if (ui_repeat(BTN_UP))
          cur_idx = (cur_idx <= 0) ? nf - 1 : cur_idx - 1;
        else
          cur_idx = (cur_idx + 1) % nf;
        audio_rebuild_instrument(ui->engine, (uint8_t)ui->ctx_instrument);
        tracker_inst_set_slot(inst, slot, defs[cur_idx]->id);
      }
      if (input_pressed(BTN_B) && sl->unit_id[0])
        sl->enabled = !sl->enabled;
    }
  } else {
    int slot = ui->ctx_instrument_slot;
    const UnitDef *def = slot_def(inst, slot);
    if (!def) {
      ui->inst_row = slot;
      return;
    }

    int nparams = def->num_params;
    int data_row = CHAIN_MAX + nparams;
    bool has_data = (def->data_hint != NULL);
    int param = ui->inst_row - CHAIN_MAX;
    bool on_data = has_data && (ui->inst_row == data_row);
    ChainSlot *sl = &inst->chain[slot];

    // A on FILE row: open native file dialog (desktop only)
    if (on_data && input_pressed(BTN_A)) {
#ifdef __EMSCRIPTEN__
      (void)0;  // no native dialog on web — user sets path via text input
#else
      // Parse space-separated filter patterns into array for tinyfd
      const char *fptrs[16];
      char fbuf[128] = {0};
      int fnc = 0;
      if (def->file_filter) {
        strncpy(fbuf, def->file_filter, sizeof(fbuf) - 1);
        char *tok = fbuf;
        while (*tok && fnc < 15) {
          fptrs[fnc++] = tok;
          tok = strchr(tok, ' ');
          if (!tok)
            break;
          *tok++ = '\0';
        }
      }

      const char *chosen = tinyfd_openFileDialog(
          "Select file",
          sl->data[0] ? sl->data : ui->engine->save_dir,
          fnc, fptrs, NULL, 0);

      if (chosen) {
        // Store path relative to save_dir when possible
        const char *rel = chosen;
        const char *sd = ui->engine->save_dir;
        size_t sdlen = strlen(sd);
        if (sdlen > 0 && strncmp(chosen, sd, sdlen) == 0)
          rel = chosen + sdlen;
        strncpy(sl->data, rel, sizeof(sl->data) - 1);
        sl->data[sizeof(sl->data) - 1] = '\0';
        audio_rebuild_instrument(ui->engine, (uint8_t)ui->ctx_instrument);
      }
#endif
    }

    if (!edit) {
      if (ui_repeat(BTN_UP)) {
        if (ui->inst_row > CHAIN_MAX)
          ui->inst_row--;
        else
          ui->inst_row = slot;
      }
      if (ui_repeat(BTN_DOWN)) {
        int max_row = has_data ? data_row : CHAIN_MAX + nparams - 1;
        if (ui->inst_row < max_row)
          ui->inst_row++;
      }
      if (ui_repeat(BTN_LEFT))
        ui->inst_row = slot;

      if (on_data) {
        // B: clear data field
        if (input_pressed(BTN_B)) {
          audio_rebuild_instrument(ui->engine, (uint8_t)ui->ctx_instrument);
          inst->chain[slot].data[0] = '\0';
        }
      } else if (param < nparams) {
        if (input_pressed(BTN_B))
          inst->chain[slot].params[param] = def->param_defaults[param];
      }
    } else {
      if (!on_data && param < nparams) {
        uint8_t *v = &inst->chain[slot].params[param];
        if (ui_repeat(BTN_UP))
          (*v)++;
        if (ui_repeat(BTN_DOWN))
          (*v)--;
        if (ui_repeat(BTN_RIGHT))
          *v = (uint8_t)(*v + 16 > 255 ? 255 : *v + 16);
        if (ui_repeat(BTN_LEFT))
          *v = (uint8_t)(*v < 16 ? 0 : *v - 16);
        if (input_pressed(BTN_B))
          *v = def->param_defaults[param];
      }
    }
  }

  if (ui->inst_row < CHAIN_MAX)
    ui->ctx_instrument_slot = ui->inst_row;
}

void screen_instrument_draw(UIState *ui) {
  TrackerInstrument *inst = &ui->song->instruments[ui->ctx_instrument];
  bool in_params = (ui->inst_row >= CHAIN_MAX);
  int cur_slot = in_params ? ui->ctx_instrument_slot : ui->inst_row;

  // Header
  int hy = INST_HEADER_Y;
  DrawRectangle(0, hy, WIN_W, CH_H, C_BG_ALT);
  DrawText(TextFormat("INST %02X  %s    [L/R=prev/next]",
                      ui->ctx_instrument, inst->name),
           4, hy + (CH_H - FONT_S) / 2, FONT_S, C_STATUS);
  DrawLine(0, hy + CH_H, WIN_W, hy + CH_H, C_SEP);

  // Left panel: chain slots
  for (int s = 0; s < CHAIN_MAX; s++) {
    int y = INST_CONTENT_Y + s * CH_H;
    bool cur = !in_params && (s == ui->inst_row);
    bool sel = (s == cur_slot);
    Color bg = cur ? C_CURSOR : (sel ? C_BG_ALT : (s % 2 == 0 ? C_BG_ALT : C_BG));
    DrawRectangle(0, y, PANEL_W - 1, CH_H, bg);
    DrawText(TextFormat("%d", s), 2, y + (CH_H - FONT_S) / 2, FONT_S, C_HEADER);

    ChainSlot *sl = &inst->chain[s];
    const UnitDef *def = slot_def(inst, s);
    if (def) {
      Color nc = sl->enabled ? (def->is_source ? C_NOTE : C_FX) : C_DIM;
      DrawText(def->name, 16, y + (CH_H - FONT_S) / 2, FONT_S, nc);
      DrawText(sl->enabled ? "ON" : "OF", PANEL_W - 22, y + (CH_H - FONT_S) / 2, FONT_S, nc);
    } else {
      DrawText("--", 16, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_TEXT : C_DIM);
    }
  }
  DrawLine(PANEL_W, INST_CONTENT_Y, PANEL_W, WIN_H - STATUS_H, C_SEP);

  // Right panel
  const UnitDef *def = slot_def(inst, cur_slot);
  if (!def) {
    DrawText("empty slot", PANEL_W + 4, INST_CONTENT_Y + (CH_H - FONT_S) / 2, FONT_S, C_DIM);
    DrawText("holdA+UP/DN=set unit", PANEL_W + 4, INST_CONTENT_Y + CH_H + (CH_H - FONT_S) / 2, FONT_S - 1, C_DIM);
    return;
  }

  DrawText(TextFormat("%s  %s  [>]=params",
                      def->name, def->is_source ? "SOURCE" : "EFFECT"),
           PANEL_W + 4, INST_CONTENT_Y + (CH_H - FONT_S) / 2, FONT_S, def->is_source ? C_NOTE : C_FX);

  ChainSlot *sl = &inst->chain[cur_slot];
  int bar_x = PANEL_W + 70;
  int bar_w = WIN_W - bar_x - 4;
  int nparams = def->num_params;

  // Params
  for (int pi = 0; pi < nparams; pi++) {
    int y = INST_CONTENT_Y + CH_H + pi * CH_H;
    if (y + CH_H > WIN_H - STATUS_H)
      break;
    int param_row = CHAIN_MAX + pi;
    bool cur = in_params && (param_row == ui->inst_row);
    DrawRectangle(PANEL_W, y, WIN_W - PANEL_W, CH_H, cur ? C_CURSOR : (pi % 2 == 0 ? C_BG_ALT : C_BG));
    DrawText(def->param_names[pi], PANEL_W + 4, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_TITLE : C_TEXT);
    DrawText(TextFormat("%02X", sl->params[pi]), PANEL_W + 50, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_NOTE : C_VEL);
    float frac = sl->params[pi] / 255.0f;
    DrawRectangle(bar_x, y + 3, bar_w, CH_H - 6, C_DIM);
    DrawRectangle(bar_x, y + 3, (int)(frac * bar_w), CH_H - 6, cur ? C_NOTE : C_HEADER);
  }

  // FILE row for units that load a file
  if (def->data_hint) {
    int data_row = CHAIN_MAX + nparams;
    int y = INST_CONTENT_Y + CH_H + nparams * CH_H;
    if (y + CH_H <= WIN_H - STATUS_H) {
      bool cur = in_params && (ui->inst_row == data_row);
      bool editing = cur && ui->inst_data_editing;
      Color bg = editing ? C_CURSOR2 : (cur ? C_CURSOR : (nparams % 2 == 0 ? C_BG_ALT : C_BG));
      DrawRectangle(PANEL_W, y, WIN_W - PANEL_W, CH_H, bg);
      DrawText("FILE", PANEL_W + 4, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_TITLE : C_TEXT);

      // Show path (right-justify so end is visible when long)
      // If empty, show hint grayed out
      bool using_hint = !sl->data[0];
      const char *path = using_hint ? def->data_hint : sl->data;
      int px = PANEL_W + 38;
      int pw = WIN_W - px - 4;
      // Measure and right-clip
      int cw = MeasureText("W", FONT_S);  // approx char width
      int max_chars = pw / (cw > 0 ? cw : 6);
      int plen = (int)strlen(path);
      const char *display = (plen > max_chars) ? path + (plen - max_chars) : path;
      Color tc = editing ? C_STATUS : (using_hint ? C_HEADER : (cur ? C_NOTE : C_VEL));
      DrawText(display, px, y + (CH_H - FONT_S) / 2, FONT_S, tc);

      // Blinking cursor when editing
      if (editing && (ui->blink & 16)) {
        int cx = px + MeasureText(display, FONT_S);
        DrawRectangle(cx, y + 2, 2, CH_H - 4, C_STATUS);
      }
    }
  }
}
