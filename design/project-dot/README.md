# Project Dot face study

This directory contains the approved visual study for Project Dot and the
source geometry used by the firmware implementation.

## Repository audit

- The intended target already exists as PlatformIO environment
  `esp32s3-touch-amoled-216`. Its board option is `HG_BOARD_AMOLED_216`.
- The board is the Waveshare ESP32-S3-Touch-AMOLED-2.16 (SKU 33969/33970): an
  ESP32-S3R8 with 8 MB octal PSRAM, 16 MB flash, a 480×480 CO5300 QSPI AMOLED,
  CST9220 touch, ES7210 microphones, ES8311 speaker output, AXP2101 power
  management, and a PCF85063 RTC.
- This project does not use LVGL. `firmware/core/src/ui.cpp` renders the shared
  C++17 UI through the project's `Canvas` drawing primitives into an RGB565
  framebuffer. The ESP32 display port and CO5300 startup live below that layer.
- `App::update_model()` already publishes distinct `Ready`, `Listening`,
  `Thinking`, `Responding`, and `Error` screens through `UiModel`. The renderer
  already isolates large hero artwork in `Ui::draw_hero()`.
- Setup, pairing, prompts, settings, profiles, cards, images, diagnostics,
  alarms, battery notices, camera, and the resting clock are separate screens.
  They can retain their current information and touch behavior.
- A matching desktop profile already exists as `sim-480x480`, so later UI work
  can be checked in the simulator before any physical flash.

The smallest safe implementation path after approval is a modular Project Dot
face renderer in the shared UI layer, selected only for approved everyday hero
states. It should use existing `Canvas` primitives and the existing `UiModel`
state, leaving board drivers, touch handling, audio, networking, provisioning,
and Hermes protocol code unchanged. Essential text screens should keep their
current layouts.

The physical AMOLED panel is square. These studies use the requested circular
visible aperture inside each exact 480×480 canvas; the black pixels outside a
circular casing opening are visually indistinguishable on the AMOLED. The final
implementation should confirm the real Project Dot casing aperture before any
geometry is treated as mechanically verified.

## Visual system

- Visible display: 464 px diameter, centred in a 480×480 canvas.
- Face centre: approximately `(240, 240)`.
- Default eye centres: `(150, 205)` and `(330, 205)`.
- Default eyes: solid ovals, 50×84 px.
- Default mouth: a 124 px wide, 14 px stroke omega curve.
- The face deliberately uses the display's horizontal span so its expression is
  readable on the physical 2.16-inch panel.
- Palette: only `#000000` and `#FFFFFF`.
- Motion: Listening, Thinking, Speaking, and Error remain discrete static
  expressions. Ready uses a sparse calm blink: six seconds of Neutral, then
  half-closed, closed, half-open, and Neutral frames at 100 ms each in a
  seven-second cycle. The mouth and face position do not move. The device's
  **Animation** setting disables the blink.

The white area outside the black circle represents casing context, not display
content. Labels appear only outside the simulated display on the presentation
sheets.

## Proposed V1 state mapping

| Device state | Expression | Recommendation |
|---|---|---|
| Ready / normal idle | Idle / Neutral | Core V1 default; shown most of the time |
| Listening / recording | Listening | Core V1; attentive eyes and one small recording dot |
| Waiting for Hermes | Thinking | Core V1; raised, slightly angled eyes and a thoughtful mouth |
| Speaking audio | Speaking | Core V1; static open mouth, with no mouth animation |
| Fatal or recoverable error hero | Error / Confused | Core V1 companion; retain the existing diagnostic text |
| Successful completion | Happy | Optional V1; use only if an existing stable transition can select it without adding timers |
| Confirmation prompt | Curious | Reserve; prompts must keep their question and answer controls visible |
| Resting clock / future doze face | Sleepy | Reserve; do not displace the clock or power information |

The recommended first implementation set is **Idle, Listening, Thinking,
Speaking, and Error / Confused**. Happy is the first optional addition. Curious
and Sleepy are included for visual review but should remain reserved until their
screen behavior is agreed.

## Files for review

- `comparison-sheet.svg` — all eight expressions in white casing context.
- `default-preview.svg` — large preview of the proposed everyday face.
- `comparison-sheet.png` and `default-preview.png` — browser-friendly rendered
  copies of the two presentation sheets.
- `idle-neutral.svg`, `happy.svg`, `curious.svg`, `listening.svg`,
  `thinking.svg`, `speaking.svg`, `sleepy.svg`, and `error-confused.svg` —
  individual exact 480×480 mockups.

## Approved V1

The enlarged-eye direction and the five-expression V1 subset were approved.
The firmware uses Neutral, Listening, Thinking, Speaking, and Error / Confused
with the geometry above. The later calm-idle request adds only the Ready blink
sequence described above. Happy, Curious, and Sleepy remain design reserves.
