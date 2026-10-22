#pragma once

namespace Magma {
enum class EventType {
  // Mouse
  CLICK = 0,
  DOUBLE_CLICK = 1,
  MOUSE_UP = 2,
  MOUSE_DOWN = 3,
  MOUSE_MOVE = 4,
  MOUSE_ENTER = 5,
  MOUSE_LEAVE = 6,
  MOUSE_HOVER = 7,
  MOUSE_OUT = 8,
  CONTEXT_MENU = 9,
  SCROLL = 10,

  // Keyboard & Text
  KEY_PRESSED = 11,
  KEY_RELEASED = 12,
  TEXT_INPUT = 13,

  // Focus
  FOCUS_GAINED = 20,
  FOCUS_LOST = 21,

  // Drag and Drop
  DRAG_START = 30,
  DROP = 31,

  // General
  ELEMENT_CHANGED = 40,
  CHANGED = 41,
  RESIZE = 42,
};

class MEventHandler {
public:
private:
};
};
