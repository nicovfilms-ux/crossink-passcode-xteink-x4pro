#pragma once

#include <FreeInkUIGfxRenderer.h>
#include <GfxRenderer.h>
#include <MappedInputManager.h>

#include "activities/Activity.h"

class LostAndFoundActivity final : public Activity {
 public:
  explicit LostAndFoundActivity(GfxRenderer& renderer, MappedInputManager& mappedInput);
  ~LostAndFoundActivity() override = default;

  void onEnter() override;
  void loop() override;
  void render(RenderLock&& lock) override;

  bool preventAutoSleep() override { return false; }
  bool allowGlobalHomeGesture() const override { return false; }
  bool blocksGlobalInput() const override { return true; }

 private:
  struct ButtonRect {
    int x;
    int y;
    int w;
    int h;
  };
  ButtonRect closeBtn_{};
};
