#include <algorithm>
#include <cstdint>
#include <vector>

#include "check.hpp"
#include "hg/canvas.hpp"
#include "hg/project_dot.hpp"
#include "hg/ui.hpp"

namespace {

constexpr int kSize = 480;
constexpr uint16_t kWhite = hg::rgb565(255, 255, 255);

struct Surface {
  std::vector<uint16_t> pixels = std::vector<uint16_t>(kSize * kSize, 0);
  hg::Canvas canvas{pixels.data(), kSize, kSize, false};

  uint16_t at(int x, int y) const { return pixels[static_cast<size_t>(y * kSize + x)]; }
  void draw(hg::project_dot::Expression expression) {
    std::fill(pixels.begin(), pixels.end(), 0);
    hg::project_dot::draw(canvas, 0, 0, kSize, expression, kWhite);
  }
  bool monochrome() const {
    return std::all_of(pixels.begin(), pixels.end(), [](uint16_t px) { return px == 0 || px == kWhite; });
  }
};

struct DotDisplay : hg::Display {
  hg::DisplayInfo d;
  std::vector<uint16_t> pixels = std::vector<uint16_t>(kSize * kSize, 0x1234);
  int flushes = 0;

  DotDisplay() {
    d.width = d.height = kSize;
    d.emissive = true;
    d.theme = hg::UiTheme::ProjectDot;
  }
  hg::DisplayInfo info() const override { return d; }
  uint16_t* framebuffer() override { return pixels.data(); }
  void flush(uint16_t, uint16_t) override { ++flushes; }
  uint16_t at(int x, int y) const { return pixels[static_cast<size_t>(y * kSize + x)]; }
};

hg::UiModel everyday(hg::Screen screen) {
  hg::UiModel m;
  m.screen = screen;
  m.link = hg::Link::Online;
  m.hero = true;
  m.headline = "Dot";
  m.name_target = true;
  return m;
}

}  // namespace

TEST("project dot: neutral matches the approved large-eye geometry") {
  Surface s;
  s.draw(hg::project_dot::Expression::Neutral);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 205), kWhite);
  CHECK_EQ(s.at(330, 205), kWhite);
  CHECK_EQ(s.at(240, 205), uint16_t(0));
  CHECK_EQ(s.at(240, 274), kWhite);
  CHECK_EQ(s.at(100, 205), uint16_t(0));
  CHECK_EQ(s.at(380, 205), uint16_t(0));
}

TEST("project dot: listening, thinking, speaking and confused are discrete static expressions") {
  Surface s;

  s.draw(hg::project_dot::Expression::Listening);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 202), kWhite);
  CHECK_EQ(s.at(240, 335), kWhite);
  CHECK_EQ(s.at(240, 315), uint16_t(0));

  s.draw(hg::project_dot::Expression::Thinking);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(155, 195), kWhite);
  CHECK_EQ(s.at(335, 174), kWhite);
  CHECK_EQ(s.at(240, 205), uint16_t(0));

  s.draw(hg::project_dot::Expression::Speaking);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 205), kWhite);
  CHECK_EQ(s.at(240, 251), kWhite);
  CHECK_EQ(s.at(240, 282), uint16_t(0));

  s.draw(hg::project_dot::Expression::Confused);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 202), kWhite);
  CHECK_EQ(s.at(331, 200), kWhite);
  CHECK_EQ(s.at(240, 202), uint16_t(0));
}

TEST("project dot: calm idle transition frames close only the eyes") {
  Surface s;

  s.draw(hg::project_dot::Expression::IdleHalfBlink);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 205), kWhite);
  CHECK_EQ(s.at(330, 205), kWhite);
  CHECK_EQ(s.at(150, 170), uint16_t(0));
  CHECK_EQ(s.at(240, 274), kWhite);

  s.draw(hg::project_dot::Expression::IdleBlink);
  CHECK(s.monochrome());
  CHECK_EQ(s.at(150, 211), kWhite);
  CHECK_EQ(s.at(330, 211), kWhite);
  CHECK_EQ(s.at(150, 170), uint16_t(0));
  CHECK_EQ(s.at(240, 274), kWhite);
}

TEST("project dot: canonical geometry scales down without adding colours") {
  std::vector<uint16_t> pixels(240 * 240, 0);
  hg::Canvas canvas(pixels.data(), 240, 240, false);
  hg::project_dot::draw(canvas, 0, 0, 240, hg::project_dot::Expression::Neutral, kWhite);
  CHECK(std::all_of(pixels.begin(), pixels.end(), [](uint16_t px) { return px == 0 || px == kWhite; }));
  CHECK_EQ(pixels[102 * 240 + 75], kWhite);
  CHECK_EQ(pixels[102 * 240 + 165], kWhite);
  CHECK_EQ(pixels[137 * 240 + 120], kWhite);
}

TEST("project dot UI: approved online states own the whole screen") {
  DotDisplay display;
  hg::Ui ui(display);
  hg::UiModel m = everyday(hg::Screen::Ready);
  hg::mascot::Face custom_face;
  custom_face.hash = "profile-face";
  m.face = &custom_face;  // the grid keeps this; Project Dot owns the everyday screen
  ui.render(m);
  CHECK_EQ(display.at(150, 205), kWhite);
  CHECK_EQ(display.at(0, 0), uint16_t(0));
  CHECK(!ui.name_hit(m, 240, 350));
  CHECK(ui.title_hit(240, 1));
  const int flushed = display.flushes;
  m.frame = 99;
  m.level = 100;
  ui.render(m);
  CHECK_EQ(display.flushes, flushed);

  m.screen = hg::Screen::Listening;
  ui.render(m);
  CHECK_EQ(display.at(240, 335), kWhite);
  m.screen = hg::Screen::Thinking;
  ui.render(m);
  CHECK_EQ(display.at(335, 174), kWhite);
  m.screen = hg::Screen::Responding;
  m.speaking = true;
  ui.render(m);
  CHECK_EQ(display.at(240, 282), uint16_t(0));
}

TEST("project dot UI: operational screens and timers keep the conventional interface") {
  DotDisplay display;
  hg::Ui ui(display);
  hg::UiModel m = everyday(hg::Screen::Ready);
  m.timer_permille = 500;
  ui.render(m);
  CHECK(display.at(0, 0) != uint16_t(0));
  CHECK(ui.name_hit(m, 240, 350));

  m.timer_permille = -1;
  m.link = hg::Link::Offline;
  ui.render(m);
  CHECK(display.at(0, 0) != uint16_t(0));

  m.link = hg::Link::Online;
  m.screen = hg::Screen::Error;
  m.detail = "Still readable";
  ui.render(m);
  CHECK(std::count(display.pixels.begin(), display.pixels.end(), kWhite) > 500);
  CHECK(display.at(0, 0) != uint16_t(0));
}

TEST("project dot UI: Ready blinks sparsely without redundant repaints") {
  DotDisplay display;
  hg::Ui ui(display);
  hg::UiModel m = everyday(hg::Screen::Ready);
  m.frame = 59;
  ui.render(m);
  const int neutral_flushes = display.flushes;

  m.frame = 60;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 1);
  CHECK_EQ(display.at(150, 170), uint16_t(0));
  m.frame = 61;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 2);
  CHECK_EQ(display.at(150, 211), kWhite);
  m.frame = 62;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 3);
  m.frame = 63;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 4);
  CHECK_EQ(display.at(150, 170), kWhite);

  for (uint32_t frame : {64u, 69u, 70u, 119u}) {
    m.frame = frame;
    ui.render(m);
  }
  CHECK_EQ(display.flushes, neutral_flushes + 4);

  m.frame = 60;
  m.animate = false;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 4);
  m.screen = hg::Screen::Responding;
  m.animate = true;
  m.speaking = false;
  ui.render(m);
  CHECK_EQ(display.flushes, neutral_flushes + 4);
}
