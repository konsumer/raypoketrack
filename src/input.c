#include "input.h"

#include <string.h>

#include "raylib.h"

// RetroArch SNES keyboard defaults
static const int KB_MAP[BTN_COUNT] = {
    [BTN_UP] = KEY_UP,
    [BTN_DOWN] = KEY_DOWN,
    [BTN_LEFT] = KEY_LEFT,
    [BTN_RIGHT] = KEY_RIGHT,
    [BTN_A] = KEY_X,
    [BTN_B] = KEY_Z,
    [BTN_X] = KEY_S,
    [BTN_Y] = KEY_A,
    [BTN_L] = KEY_Q,
    [BTN_R] = KEY_W,
    [BTN_START] = KEY_ENTER,
    [BTN_SELECT] = KEY_RIGHT_SHIFT,
};

// Standard gamepad layout (SNES/USB gamepad)
static const int GP_MAP[BTN_COUNT] = {
    [BTN_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP,
    [BTN_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    [BTN_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    [BTN_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    [BTN_A] = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    [BTN_B] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    [BTN_X] = GAMEPAD_BUTTON_RIGHT_FACE_UP,
    [BTN_Y] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    [BTN_L] = GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    [BTN_R] = GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    [BTN_START] = GAMEPAD_BUTTON_MIDDLE_RIGHT,
    [BTN_SELECT] = GAMEPAD_BUTTON_MIDDLE_LEFT,
};

static bool prev[BTN_COUNT];
static bool curr[BTN_COUNT];
static int hframes[BTN_COUNT];

static bool raw_down(TrackerButton btn) {
  if (IsKeyDown(KB_MAP[btn]))
    return true;
  if (btn == BTN_SELECT && IsKeyDown(KEY_LEFT_SHIFT))
    return true;
  if (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GP_MAP[btn]))
    return true;
  return false;
}

void input_update(void) {
  memcpy(prev, curr, sizeof(curr));
  for (int i = 0; i < BTN_COUNT; i++) {
    curr[i] = raw_down((TrackerButton)i);
    hframes[i] = curr[i] ? hframes[i] + 1 : 0;
  }
}

bool input_pressed(TrackerButton btn) { return curr[btn] && !prev[btn]; }
bool input_released(TrackerButton btn) { return !curr[btn] && prev[btn]; }
bool input_held(TrackerButton btn) { return curr[btn]; }
int input_held_frames(TrackerButton btn) { return hframes[btn]; }
