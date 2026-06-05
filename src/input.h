#pragma once
#include <stdbool.h>

// All physical buttons mapped to logical tracker buttons
// Keyboard defaults match RetroArch SNES layout
typedef enum {
  BTN_UP = 0,
  BTN_DOWN,
  BTN_LEFT,
  BTN_RIGHT,
  BTN_A,       // X key / gamepad east  — confirm, enter
  BTN_B,       // Z key / gamepad south — back, delete
  BTN_X,       // S key / gamepad north — alt
  BTN_Y,       // A key / gamepad west  — alt
  BTN_L,       // Q key / left shoulder
  BTN_R,       // W key / right shoulder
  BTN_START,   // Enter / gamepad middle-right
  BTN_SELECT,  // RShift / gamepad middle-left
  BTN_COUNT,
} TrackerButton;

void input_update(void);
bool input_pressed(TrackerButton btn);     // went down this frame
bool input_released(TrackerButton btn);    // went up this frame
bool input_held(TrackerButton btn);        // currently down
int input_held_frames(TrackerButton btn);  // frames held (0 if not held)
