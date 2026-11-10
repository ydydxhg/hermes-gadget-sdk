#include "hg/project_dot.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>

namespace hg::project_dot {
namespace {

constexpr int kCanonical = 480;
constexpr int kTrig = 1024;

struct Point {
  int x, y;
};

int scaled(int value, int size) { return (value * size + kCanonical / 2) / kCanonical; }

Point point(int x, int y, int ox, int oy, int size) {
  return {ox + scaled(x, size), oy + scaled(y, size)};
}

void ellipse(Canvas& c, int ox, int oy, int size, int cx, int cy, int rx, int ry, uint16_t ink,
             int cos_a = kTrig, int sin_a = 0) {
  const Point center = point(cx, cy, ox, oy, size);
  const int ax = std::max(1, scaled(rx, size));
  const int ay = std::max(1, scaled(ry, size));
  const int reach = std::max(ax, ay) + 1;
  const int64_t ax2 = static_cast<int64_t>(ax) * ax;
  const int64_t ay2 = static_cast<int64_t>(ay) * ay;
  const int64_t limit = ax2 * ay2 * kTrig * kTrig;
  for (int dy = -reach; dy <= reach; ++dy) {
    for (int dx = -reach; dx <= reach; ++dx) {
      const int64_t xr = static_cast<int64_t>(dx) * cos_a + static_cast<int64_t>(dy) * sin_a;
      const int64_t yr = -static_cast<int64_t>(dx) * sin_a + static_cast<int64_t>(dy) * cos_a;
      if (xr * xr * ay2 + yr * yr * ax2 <= limit) c.pixel(center.x + dx, center.y + dy, ink);
    }
  }
}

Point quadratic(Point p0, Point p1, Point p2, int step, int steps) {
  const int64_t u = steps - step;
  const int64_t den = static_cast<int64_t>(steps) * steps;
  return {static_cast<int>((u * u * p0.x + 2 * u * step * p1.x + static_cast<int64_t>(step) * step * p2.x + den / 2) / den),
          static_cast<int>((u * u * p0.y + 2 * u * step * p1.y + static_cast<int64_t>(step) * step * p2.y + den / 2) / den)};
}

Point cubic(Point p0, Point p1, Point p2, Point p3, int step, int steps) {
  const int64_t u = steps - step;
  const int64_t s = step;
  const int64_t den = static_cast<int64_t>(steps) * steps * steps;
  return {static_cast<int>((u * u * u * p0.x + 3 * u * u * s * p1.x + 3 * u * s * s * p2.x + s * s * s * p3.x + den / 2) / den),
          static_cast<int>((u * u * u * p0.y + 3 * u * u * s * p1.y + 3 * u * s * s * p2.y + s * s * s * p3.y + den / 2) / den)};
}

void stamp(Canvas& c, Point p, int radius, uint16_t ink) { c.fill_circle(p.x, p.y, radius, ink); }

void quadratic_stroke(Canvas& c, int ox, int oy, int size, Point p0, Point p1, Point p2, int width,
                      uint16_t ink) {
  const int radius = std::max(1, scaled(width, size) / 2);
  p0 = point(p0.x, p0.y, ox, oy, size);
  p1 = point(p1.x, p1.y, ox, oy, size);
  p2 = point(p2.x, p2.y, ox, oy, size);
  constexpr int kSteps = 48;
  for (int i = 0; i <= kSteps; ++i) stamp(c, quadratic(p0, p1, p2, i, kSteps), radius, ink);
}

void cubic_stroke(Canvas& c, int ox, int oy, int size, Point p0, Point p1, Point p2, Point p3, int width,
                  uint16_t ink) {
  const int radius = std::max(1, scaled(width, size) / 2);
  p0 = point(p0.x, p0.y, ox, oy, size);
  p1 = point(p1.x, p1.y, ox, oy, size);
  p2 = point(p2.x, p2.y, ox, oy, size);
  p3 = point(p3.x, p3.y, ox, oy, size);
  constexpr int kSteps = 64;
  for (int i = 0; i <= kSteps; ++i) stamp(c, cubic(p0, p1, p2, p3, i, kSteps), radius, ink);
}

void line(Canvas& c, int ox, int oy, int size, Point p0, Point p1, int width, uint16_t ink) {
  p0 = point(p0.x, p0.y, ox, oy, size);
  p1 = point(p1.x, p1.y, ox, oy, size);
  const int radius = std::max(1, scaled(width, size) / 2);
  const int steps = std::max(std::abs(p1.x - p0.x), std::abs(p1.y - p0.y));
  for (int i = 0; i <= steps; ++i) {
    const int x = steps ? p0.x + (p1.x - p0.x) * i / steps : p0.x;
    const int y = steps ? p0.y + (p1.y - p0.y) * i / steps : p0.y;
    stamp(c, {x, y}, radius, ink);
  }
}

void omega_mouth(Canvas& c, int x, int y, int size, uint16_t ink) {
  cubic_stroke(c, x, y, size, {178, 270}, {192, 300}, {224, 300}, {240, 274}, 14, ink);
  cubic_stroke(c, x, y, size, {240, 274}, {256, 300}, {288, 300}, {302, 270}, 14, ink);
}

void neutral(Canvas& c, int x, int y, int size, uint16_t ink) {
  ellipse(c, x, y, size, 150, 205, 25, 42, ink);
  ellipse(c, x, y, size, 330, 205, 25, 42, ink);
  omega_mouth(c, x, y, size, ink);
}

}  // namespace

void draw(Canvas& c, int x, int y, int size, Expression expression, uint16_t ink) {
  switch (expression) {
    case Expression::Neutral:
      neutral(c, x, y, size, ink);
      break;
    case Expression::IdleHalfBlink:
      ellipse(c, x, y, size, 150, 205, 25, 14, ink);
      ellipse(c, x, y, size, 330, 205, 25, 14, ink);
      omega_mouth(c, x, y, size, ink);
      break;
    case Expression::IdleBlink:
      quadratic_stroke(c, x, y, size, {125, 205}, {150, 217}, {175, 205}, 10, ink);
      quadratic_stroke(c, x, y, size, {305, 205}, {330, 217}, {355, 205}, 10, ink);
      omega_mouth(c, x, y, size, ink);
      break;
    case Expression::Listening:
      ellipse(c, x, y, size, 150, 202, 26, 46, ink);
      ellipse(c, x, y, size, 330, 202, 26, 46, ink);
      quadratic_stroke(c, x, y, size, {216, 280}, {240, 295}, {264, 280}, 12, ink);
      c.fill_circle(x + scaled(240, size), y + scaled(335, size), std::max(1, scaled(10, size)), ink);
      break;
    case Expression::Thinking:
      // cos(-18 deg), sin(-18 deg), scaled by kTrig.
      ellipse(c, x, y, size, 155, 195, 23, 40, ink, 974, -316);
      ellipse(c, x, y, size, 335, 174, 23, 40, ink, 974, -316);
      quadratic_stroke(c, x, y, size, {205, 286}, {235, 265}, {265, 286}, 12, ink);
      break;
    case Expression::Speaking:
      ellipse(c, x, y, size, 150, 205, 25, 42, ink);
      ellipse(c, x, y, size, 330, 205, 25, 42, ink);
      ellipse(c, x, y, size, 240, 282, 29, 38, ink);
      ellipse(c, x, y, size, 240, 282, 15, 24, 0);
      break;
    case Expression::Confused:
      ellipse(c, x, y, size, 150, 202, 25, 42, ink);
      line(c, x, y, size, {295, 212}, {367, 188}, 18, ink);
      quadratic_stroke(c, x, y, size, {182, 288}, {210, 258}, {238, 288}, 12, ink);
      quadratic_stroke(c, x, y, size, {238, 288}, {266, 318}, {300, 284}, 12, ink);
      break;
  }
}

}  // namespace hg::project_dot
