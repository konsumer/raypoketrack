#include "ui.h"

#define SONG_LABEL_W 28
#define SONG_HEADER_Y (STATUS_H + 1)
#define SONG_CONTENT_Y (SONG_HEADER_Y + CH_H + 1)
#define SONG_VISIBLE ((WIN_H - SONG_CONTENT_Y - STATUS_H) / CH_H)
#define CELL_W ((WIN_W - SONG_LABEL_W) / SONG_VIEW_COLS)

// X position of channel `ch` relative to scroll
static int chan_x(UIState *ui, int ch) {
  return SONG_LABEL_W + (ch - ui->song_col_scroll) * CELL_W;
}

static uint8_t next_free_pattern(TrackerSong *song) {
  bool used[NUM_PATTERNS] = {0};
  for (int ch = 0; ch < SONG_CHANNELS; ch++)
    for (int r = 0; r < SONG_LENGTH; r++)
      if (song->patterns[ch][r] != TRACKER_EMPTY)
        used[song->patterns[ch][r]] = true;
  for (int i = 0; i < NUM_PATTERNS; i++)
    if (!used[i])
      return (uint8_t)i;
  return TRACKER_EMPTY;
}

void screen_song_update(UIState *ui) {
  TrackerSong *song = ui->song;
  bool edit = input_held(BTN_A);

  if (!edit) {
    // Row navigation
    if (ui_repeat(BTN_DOWN) && ui->song_row < SONG_LENGTH - 1) {
      ui->song_row++;
      if (ui->song_row >= ui->song_scroll + SONG_VISIBLE)
        ui->song_scroll = ui->song_row - SONG_VISIBLE + 1;
    }
    if (ui_repeat(BTN_UP) && ui->song_row > 0) {
      ui->song_row--;
      if (ui->song_row < ui->song_scroll)
        ui->song_scroll = ui->song_row;
    }

    // Channel navigation with horizontal scroll
    if (ui_repeat(BTN_RIGHT) && ui->song_col < SONG_CHANNELS - 1) {
      ui->song_col++;
      if (ui->song_col >= ui->song_col_scroll + SONG_VIEW_COLS)
        ui->song_col_scroll = ui->song_col - SONG_VIEW_COLS + 1;
    }
    if (ui_repeat(BTN_LEFT) && ui->song_col > 0) {
      ui->song_col--;
      if (ui->song_col < ui->song_col_scroll)
        ui->song_col_scroll = ui->song_col;
    }

    // L/R shoulder: jump 8 rows
    if (ui_repeat(BTN_R))
      ui->song_row = (ui->song_row + 8 < SONG_LENGTH) ? ui->song_row + 8 : ui->song_row;
    if (ui_repeat(BTN_L))
      ui->song_row = (ui->song_row >= 8) ? ui->song_row - 8 : 0;

    // Clamp scroll
    if (ui->song_row < ui->song_scroll)
      ui->song_scroll = ui->song_row;
    if (ui->song_row >= ui->song_scroll + SONG_VISIBLE)
      ui->song_scroll = ui->song_row - SONG_VISIBLE + 1;

    // B: clear pattern at cursor
    if (input_pressed(BTN_B))
      song->patterns[ui->song_col][ui->song_row] = TRACKER_EMPTY;

  } else {
    // A + UP/DOWN: change pattern index at cursor
    uint8_t *cell = &song->patterns[ui->song_col][ui->song_row];
    if (ui_repeat(BTN_UP)) {
      if (*cell == TRACKER_EMPTY)
        *cell = 0;
      else if (*cell < NUM_PATTERNS - 1)
        (*cell)++;
    }
    if (ui_repeat(BTN_DOWN)) {
      if (*cell == TRACKER_EMPTY)
        *cell = 0;
      else if (*cell > 0)
        (*cell)--;
    }
    if (input_pressed(BTN_B))
      *cell = TRACKER_EMPTY;
  }

  // Update context from cursor
  ui->ctx_channel = ui->song_col;
  uint8_t pi = song->patterns[ui->song_col][ui->song_row];
  if (pi != TRACKER_EMPTY)
    ui->ctx_pattern = pi;
}

void screen_song_draw(UIState *ui) {
  TrackerSong *song = ui->song;

  // Header
  DrawRectangle(0, SONG_HEADER_Y, WIN_W, CH_H, C_BG_ALT);
  DrawText("ROW", 2, SONG_HEADER_Y + (CH_H - FONT_S) / 2, FONT_S, C_HEADER);

  // Channel labels for visible range
  for (int i = 0; i < SONG_VIEW_COLS; i++) {
    int ch = ui->song_col_scroll + i;
    if (ch >= SONG_CHANNELS)
      break;
    int x = SONG_LABEL_W + i * CELL_W;
    bool cur_ch = (ch == ui->song_col);
    DrawText(TextFormat("%X", ch), x + 2, SONG_HEADER_Y + (CH_H - FONT_S) / 2, FONT_S,
             cur_ch ? C_TITLE : CH_COLORS[ch]);
  }
  // Scroll indicators
  if (ui->song_col_scroll > 0)
    DrawText("<", SONG_LABEL_W - 8, SONG_HEADER_Y + (CH_H - FONT_S) / 2, FONT_S, C_DIM);
  if (ui->song_col_scroll + SONG_VIEW_COLS < SONG_CHANNELS)
    DrawText(">", WIN_W - 8, SONG_HEADER_Y + (CH_H - FONT_S) / 2, FONT_S, C_DIM);

  DrawLine(0, SONG_HEADER_Y + CH_H, WIN_W, SONG_HEADER_Y + CH_H, C_SEP);

  // Rows
  for (int vi = 0; vi < SONG_VISIBLE; vi++) {
    int r = ui->song_scroll + vi;
    if (r >= SONG_LENGTH)
      break;
    int y = SONG_CONTENT_Y + vi * CH_H;

    Color row_bg = (r % 4 == 0) ? C_BG_ALT : C_BG;
    if (audio_is_playing(ui->engine)) {
      for (int ch = 0; ch < SONG_CHANNELS; ch++) {
        if (ui->engine->cursors[ch].song_row == r) {
          row_bg = C_CURSOR2;
          break;
        }
      }
    }
    DrawRectangle(0, y, WIN_W, CH_H, row_bg);
    DrawText(TextFormat("%02X", r), 2, y + (CH_H - FONT_S) / 2, FONT_S,
             r == ui->song_row ? C_TITLE : C_HEADER);

    for (int i = 0; i < SONG_VIEW_COLS; i++) {
      int ch = ui->song_col_scroll + i;
      if (ch >= SONG_CHANNELS)
        break;
      int x = SONG_LABEL_W + i * CELL_W;
      bool cur = (r == ui->song_row && ch == ui->song_col);
      uint8_t pi = song->patterns[ch][r];

      bool playing_cell = audio_is_playing(ui->engine) && ui->engine->cursors[ch].song_row == r;
      Color cell_bg;
      Color text_col;
      const char *label;

      if (pi == TRACKER_EMPTY) {
        label = "--";
        text_col = C_DIM;
      } else {
        label = TextFormat("%02X", pi);
        text_col = CH_COLORS[ch];
      }
      if (playing_cell) {
        cell_bg = (Color){0x10, 0x40, 0x20, 0xFF};
        text_col = C_PLAY;
      } else {
        cell_bg = BLANK;
      }
      // Cursor always wins — draw over playing indicator
      if (cur) {
        cell_bg = C_CURSOR;
        text_col = C_TITLE;
      }
      draw_cell(x, y, CELL_W - 1, CH_H, cell_bg, label, FONT_S, text_col);
    }
  }

  // Scrollbar (vertical)
  if (SONG_LENGTH > SONG_VISIBLE) {
    int bar_h = WIN_H - SONG_CONTENT_Y - STATUS_H;
    int th = bar_h * SONG_VISIBLE / SONG_LENGTH;
    int ty = SONG_CONTENT_Y + bar_h * ui->song_scroll / SONG_LENGTH;
    DrawRectangle(WIN_W - 3, SONG_CONTENT_Y, 3, bar_h, C_DIM);
    DrawRectangle(WIN_W - 3, ty, 3, th, C_HEADER);
  }
}
