#include <stdio.h>
#include <string.h>

#include "tracker.h"
#include "ui.h"

#define MENU_CONTENT_Y (STATUS_H + 2)
#define SONG_FILE "song.rpt"

typedef enum {
  MENU_BPM = 0,
  MENU_SCALE_ROOT,
  MENU_SCALE,
  MENU_SAVE,
  MENU_LOAD,
  MENU_NEW,
  MENU_COUNT,
} MenuItem;

static const char *menu_labels[] = {
    "BPM",
    "KEY",
    "SCALE",
    "SAVE",
    "LOAD",
    "NEW SONG",
};

static char status_msg[48] = "";
static int status_timer = 0;

void screen_menu_update(UIState *ui) {
  bool edit = input_held(BTN_A);

  if (status_timer > 0)
    status_timer--;

  if (!edit) {
    if (ui_repeat(BTN_UP) && ui->menu_row > 0)
      ui->menu_row--;
    if (ui_repeat(BTN_DOWN) && ui->menu_row < MENU_COUNT - 1)
      ui->menu_row++;
  } else {
    switch (ui->menu_row) {
      case MENU_BPM:
        if (ui_repeat(BTN_UP) && ui->song->bpm < 250)
          ui->song->bpm++;
        if (ui_repeat(BTN_DOWN) && ui->song->bpm > 40)
          ui->song->bpm--;
        if (ui_repeat(BTN_RIGHT) && ui->song->bpm <= 240)
          ui->song->bpm += 10;
        if (ui_repeat(BTN_LEFT) && ui->song->bpm >= 50)
          ui->song->bpm -= 10;
        break;

      case MENU_SCALE_ROOT:
        if (ui_repeat(BTN_UP))
          ui->song->scale_root = (ui->song->scale_root + 1) % 12;
        if (ui_repeat(BTN_DOWN))
          ui->song->scale_root = (ui->song->scale_root + 11) % 12;
        break;

      case MENU_SCALE:
        if (ui_repeat(BTN_UP))
          ui->song->scale_idx = (ui->song->scale_idx + 1) % NUM_SCALES;
        if (ui_repeat(BTN_DOWN))
          ui->song->scale_idx = (ui->song->scale_idx + NUM_SCALES - 1) % NUM_SCALES;
        break;

      case MENU_SAVE:
        if (input_pressed(BTN_A)) {
          bool ok = tracker_save(ui->song, SONG_FILE);
          if (ok)
            audio_set_save_dir(ui->engine, SONG_FILE);
          snprintf(status_msg, sizeof(status_msg),
                   ok ? "SAVED: %s" : "SAVE FAILED: %s", SONG_FILE);
          status_timer = 180;
        }
        break;

      case MENU_LOAD:
        if (input_pressed(BTN_A)) {
          audio_stop(ui->engine);
          TrackerSong tmp;
          if (tracker_load(&tmp, SONG_FILE)) {
            *ui->song = tmp;
            audio_set_save_dir(ui->engine, SONG_FILE);
            snprintf(status_msg, sizeof(status_msg), "LOADED: %s", SONG_FILE);
          } else {
            snprintf(status_msg, sizeof(status_msg), "LOAD FAILED: %s", SONG_FILE);
          }
          status_timer = 180;
        }
        break;

      case MENU_NEW:
        if (input_pressed(BTN_A)) {
          audio_stop(ui->engine);
          tracker_clear(ui->song);
          ui->song_row = ui->song_col = 0;
          ui->pattern_row = ui->pattern_col = 0;
          ui->inst_row = ui->song_scroll = ui->song_col_scroll = 0;
          ui->ctx_pattern = ui->ctx_instrument = 0;
          snprintf(status_msg, sizeof(status_msg), "NEW SONG");
          status_timer = 120;
        }
        break;
    }
  }
}

void screen_menu_draw(UIState *ui) {
  for (int i = 0; i < MENU_COUNT; i++) {
    int y = MENU_CONTENT_Y + i * (CH_H + 2);
    bool cur = (i == ui->menu_row);
    DrawRectangle(0, y, WIN_W, CH_H, cur ? C_CURSOR : (i % 2 == 0 ? C_BG_ALT : C_BG));
    DrawText(menu_labels[i], 4, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_TITLE : C_HEADER);

    char val[64] = "";
    switch (i) {
      case MENU_BPM:
        snprintf(val, sizeof(val), "%d", ui->song->bpm);
        DrawText(val, 100, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_NOTE : C_TEXT);
        break;
      case MENU_SCALE_ROOT:
        DrawText(SCALE_ROOT_NAMES[ui->song->scale_root % 12],
                 100, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_NOTE : C_TEXT);
        break;
      case MENU_SCALE:
        DrawText(SCALES[ui->song->scale_idx % NUM_SCALES].name,
                 100, y + (CH_H - FONT_S) / 2, FONT_S, cur ? C_NOTE : C_TEXT);
        break;
      case MENU_SAVE:
      case MENU_LOAD:
        DrawText(SONG_FILE, 100, y + (CH_H - FONT_S) / 2, FONT_S - 1, cur ? C_DIM : C_DIM);
        if (cur) DrawText("[holdA+A]", WIN_W - 64, y + (CH_H - FONT_S) / 2, FONT_S - 1, C_DIM);
        break;
      case MENU_NEW:
        DrawText(cur ? "[holdA+A=confirm]" : "", 100, y + (CH_H - FONT_S) / 2, FONT_S - 1, C_DIM);
        break;
    }
  }

  // Status message
  if (status_timer > 0) {
    int y = MENU_CONTENT_Y + MENU_COUNT * (CH_H + 2) + CH_H;
    DrawRectangle(0, y, WIN_W, CH_H, C_BG_ALT);
    DrawText(status_msg, 4, y + (CH_H - FONT_S) / 2, FONT_S,
             strncmp(status_msg, "SAVE", 4) == 0 || strncmp(status_msg, "LOAD", 4) == 0 || strncmp(status_msg, "NEW", 3) == 0
                 ? C_PLAY
                 : C_NOTE_OFF);
  }
}
