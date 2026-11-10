# Project Dot Calm Idle Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give Project Dot's Ready face a sparse blink sequence that feels alive without animating other states or affecting device services.

**Architecture:** Reuse the app's existing 100 ms `UiModel::frame`. Map Ready frames through a 70-frame deterministic cycle and render two additional eye-only transition frames with the existing allocation-free primitive renderer; keep the mouth fixed and let the full-face expression hash suppress all unchanged repaints.

**Tech Stack:** C++17, Hermes `Canvas`, existing `UiModel`/`Ui` renderer, host C++ tests, simulator, PlatformIO ESP-IDF build.

**Spec:** `design/project-dot/README.md` plus the delegated design decision in the 2026-11-10 conversation.

## Global Constraints

- Animate only the online full-screen Ready state.
- Keep Listening, Thinking, Speaking, Error and all operational screens unchanged.
- Use only pure black and pure white; do not move the face or mouth.
- Add no dependencies, timers, tasks, heap allocation, or audio/network work.
- Cycle: Neutral frames 0–59, Half Blink 60, Blink 61, Half Blink 62, Neutral 63–69; one frame is 100 ms.
- Repaint only when the visible animation phase changes.

## Review Focus

- Frame wrap at 69 → 0 must remain Neutral and cause no redundant repaint.
- Entering Ready at any global frame must produce one valid monochrome phase.
- Responding while not speaking must remain static Neutral, not inherit the idle blink.
- Animation must not appear when `UiModel::animate` is false.
- Existing offline, timer, custom-profile-face, settings-target and fallback behavior must remain intact.

---

### Task 1: Calm idle blink

**Files:**
- Modify: `firmware/core/include/hg/project_dot.hpp`
- Modify: `firmware/core/src/project_dot.cpp`
- Modify: `firmware/core/src/ui.cpp`
- Test: `firmware/tests/test_project_dot.cpp`
- Modify: `design/project-dot/README.md`
- Modify: `docs/using-gadget.md`
- Modify: `docs/hardware.md`
- Modify: `CHANGELOG.md`

**Interfaces:**
- Consumes: `UiModel::frame`, `UiModel::animate`, `project_dot::draw(...)`.
- Produces: `Expression::IdleHalfBlink`, `Expression::IdleBlink`, and Ready-only phase selection.

- [ ] **Step 1: Write failing renderer and UI tests**

Add literal pixel assertions for half-closed and closed eyes, assert the fixed mouth remains, and assert Ready flushes exactly at frames 60, 61, 62 and 63 while ordinary Neutral frames and the 69 → 0 wrap do not flush. Assert `animate=false` and non-speaking Responding remain static Neutral.

- [ ] **Step 2: Run the Project Dot core tests and verify RED**

Run: `cmake --build build/host -j4 --target hg_core_tests`

Expected: compilation fails because the two idle transition expressions do not exist.

- [ ] **Step 3: Implement the two transition drawings and Ready phase selector**

Extend `project_dot::Expression`, draw consistent half-blink and blink eyes with the unchanged omega mouth, and make `project_dot_expression(const UiModel&)` apply the exact 70-frame cycle only to animated Ready.

- [ ] **Step 4: Verify GREEN and regressions**

Run the core test suite, the targeted `sim-480x480` framebuffer test, documentation build, and `pio run -e esp32s3-touch-amoled-216`. Confirm all pass and report simulator/build evidence separately from unperformed hardware validation.

- [ ] **Step 5: Leave changes uncommitted**

This checkout is `main` with the user's existing uncommitted Project Dot work; do not commit or rewrite that history without explicit instruction.
