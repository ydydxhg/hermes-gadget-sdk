// Project Dot's static monochrome expressions, drawn from geometric primitives.
#pragma once

#include <cstdint>

#include "hg/canvas.hpp"

namespace hg::project_dot {

enum class Expression : uint8_t { Neutral, IdleHalfBlink, IdleBlink, Listening, Thinking, Speaking, Confused };

// Draws one expression into a square. Coordinates scale from the approved
// 480x480 mockups; the caller owns and clears the background.
void draw(Canvas& canvas, int x, int y, int size, Expression expression, uint16_t ink);

}  // namespace hg::project_dot
