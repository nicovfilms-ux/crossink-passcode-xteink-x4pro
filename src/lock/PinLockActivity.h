#pragma once

#include <FreeInkUIGfxRenderer.h>
#include <GfxRenderer.h>
#include <MappedInputManager.h>

#include <string>
#include <vector>

#include "activities/Activity.h"
#include "lock/LockManager.h"
#include "lock/PinStorage.h"

class PinLockActivity final : public Activity {
 public:
  enum class Mode : uint8_t {
    Unlock,
    SetPin,
    ChangePin,
    VerifyForDisable
  };

  enum class ResumeTarget : uint8_t {
    None,
    Reader,
    Home
  };

  explicit PinLockActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                           Mode mode = Mode::Unlock,
                           ResumeTarget resumeTarget = ResumeTarget::None,
                           std::string resumeBookPath = "");

  ~PinLockActivity() override = default;

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&& lock) override;

  bool preventAutoSleep() override { return false; }
  bool allowGlobalHomeGesture() const override { return mode_ != Mode::Unlock; }
  bool blocksGlobalInput() const override { return mode_ == Mode::Unlock; }

 private:
  Mode mode_;
  ResumeTarget resumeTarget_;
  std::string resumeBookPath_;

  uint8_t step_ = 0;
  std::string enteredPin_;
  std::string draftPin_;
  std::string errorMessage_;
  uint32_t errorUntilMs_ = 0;
  bool needsFullRepaint_ = true;

  struct KeyRect {
    char key;  // '0'-'9', 'C' (Clear), 'E' (Enter), 'X' (Cancel)
    int x;
    int y;
    int w;
    int h;
  };
  std::vector<KeyRect> keys_;
  int dotsX_ = 0;
  int dotsY_ = 0;
  int dotsW_ = 0;
  int dotsH_ = 0;

  void setupLayout();
  void handleKeyPress(char key);
  void submitCurrentPin();
  void onUnlockSuccess();
  void renderDotsOnly();
  std::string getPromptText() const;
};
