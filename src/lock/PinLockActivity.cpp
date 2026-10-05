#include "PinLockActivity.h"

#include <FreeInkUIGfxRenderer.h>
#include <FreeInkUIIcon.h>
#include <I18n.h>

#include <algorithm>

#include "CrossPointSettings.h"
#include "Logging.h"
#include "activities/ActivityManager.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"
#include "components/icons/tablerFilledIcons.h"
#include "fontIds.h"

PinLockActivity::PinLockActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                                 const Mode mode, const ResumeTarget resumeTarget,
                                 std::string resumeBookPath)
    : Activity("PinLock", renderer, mappedInput),
      mode_(mode),
      resumeTarget_(resumeTarget),
      resumeBookPath_(std::move(resumeBookPath)) {}

void PinLockActivity::onEnter() {
  Activity::onEnter();
  step_ = 0;
  enteredPin_.clear();
  draftPin_.clear();
  errorMessage_.clear();
  errorUntilMs_ = 0;
  needsFullRepaint_ = true;
  requestUpdate();
}

void PinLockActivity::onExit() {
  Activity::onExit();
}

void PinLockActivity::setupLayout() {
  keys_.clear();
  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();

  const int keyW = (screenW > 600) ? 120 : 96;
  const int keyH = (screenH > 600) ? 64 : 52;
  const int gapX = 18;
  const int gapY = 14;
  const int totalW = 3 * keyW + 2 * gapX;
  const int totalH = 4 * keyH + 3 * gapY;
  const int startX = (screenW - totalW) / 2;

  const int startY = (screenH > 600) ? 340 : 160;

  dotsW_ = totalW;
  dotsH_ = 45;
  dotsX_ = startX;
  dotsY_ = startY - dotsH_ - 25;

  const char keyMap[4][3] = {
      {'1', '2', '3'},
      {'4', '5', '6'},
      {'7', '8', '9'},
      {'C', '0', 'E'}
  };

  for (int r = 0; r < 4; ++r) {
    for (int c = 0; c < 3; ++c) {
      keys_.push_back({
          keyMap[r][c],
          startX + c * (keyW + gapX),
          startY + r * (keyH + gapY),
          keyW,
          keyH
      });
    }
  }

  if (mode_ != Mode::Unlock) {
    const int cancelW = 110;
    const int cancelH = 40;
    keys_.push_back({
        'X',
        startX + (totalW - cancelW) / 2,
        startY + totalH + 18,
        cancelW,
        cancelH
    });
  } else {
    const int forgotW = 240;
    const int forgotH = 38;
    keys_.push_back({
        'F',
        startX + (totalW - forgotW) / 2,
        startY + totalH + 14,
        forgotW,
        forgotH
    });
  }
}

std::string PinLockActivity::getPromptText() const {
  if (!errorMessage_.empty()) {
    return errorMessage_;
  }
  switch (mode_) {
    case Mode::Unlock:
      return tr(STR_ENTER_PIN);
    case Mode::SetPin:
      return (step_ == 0) ? tr(STR_ENTER_NEW_PIN) : tr(STR_CONFIRM_NEW_PIN);
    case Mode::ChangePin:
      if (step_ == 0) return tr(STR_ENTER_CURRENT_PIN);
      if (step_ == 1) return tr(STR_ENTER_NEW_PIN);
      return tr(STR_CONFIRM_NEW_PIN);
    case Mode::VerifyForDisable:
      return tr(STR_ENTER_CURRENT_PIN);
  }
  return tr(STR_ENTER_PIN);
}

void PinLockActivity::handleKeyPress(const char key) {
  if (key >= '0' && key <= '9') {
    if (enteredPin_.size() < PinStorage::MAX_PIN_LEN) {
      enteredPin_.push_back(key);
      renderDotsOnly();
      if (mode_ == Mode::Unlock && enteredPin_.size() >= PinStorage::MIN_PIN_LEN) {
        if (PinStorage::verifyPin(enteredPin_.c_str())) {
          onUnlockSuccess();
          return;
        } else if (enteredPin_.size() == PinStorage::MAX_PIN_LEN) {
          submitCurrentPin();
          return;
        }
      }
    }
  } else if (key == 'C') {
    if (!enteredPin_.empty()) {
      enteredPin_.pop_back();
      renderDotsOnly();
    }
  } else if (key == 'E') {
    submitCurrentPin();
  } else if (key == 'X') {
    result.isCancelled = true;
    activityManager.popActivity();
  } else if (key == 'F') {
    onForgotPin();
  }
}

void PinLockActivity::submitCurrentPin() {
  if (enteredPin_.size() < PinStorage::MIN_PIN_LEN) {
    errorMessage_ = tr(STR_INCORRECT_PIN);
    errorUntilMs_ = millis() + 1500;
    enteredPin_.clear();
    needsFullRepaint_ = true;
    requestUpdate();
    return;
  }

  switch (mode_) {
    case Mode::Unlock:
      if (PinStorage::verifyPin(enteredPin_.c_str())) {
        onUnlockSuccess();
      } else {
        errorMessage_ = tr(STR_INCORRECT_PIN);
        errorUntilMs_ = millis() + 1500;
        enteredPin_.clear();
        needsFullRepaint_ = true;
        requestUpdate();
      }
      break;

    case Mode::SetPin:
      if (step_ == 0) {
        draftPin_ = enteredPin_;
        enteredPin_.clear();
        step_ = 1;
        errorMessage_.clear();
        needsFullRepaint_ = true;
        requestUpdate();
      } else {
        if (enteredPin_ == draftPin_) {
          PinStorage::setPin(enteredPin_.c_str());
          result.isCancelled = false;
          activityManager.popActivity();
        } else {
          errorMessage_ = tr(STR_PINS_DO_NOT_MATCH);
          errorUntilMs_ = millis() + 1500;
          enteredPin_.clear();
          draftPin_.clear();
          step_ = 0;
          needsFullRepaint_ = true;
          requestUpdate();
        }
      }
      break;

    case Mode::ChangePin:
      if (step_ == 0) {
        if (PinStorage::verifyPin(enteredPin_.c_str())) {
          enteredPin_.clear();
          step_ = 1;
          errorMessage_.clear();
          needsFullRepaint_ = true;
          requestUpdate();
        } else {
          errorMessage_ = tr(STR_INCORRECT_PIN);
          errorUntilMs_ = millis() + 1500;
          enteredPin_.clear();
          needsFullRepaint_ = true;
          requestUpdate();
        }
      } else if (step_ == 1) {
        draftPin_ = enteredPin_;
        enteredPin_.clear();
        step_ = 2;
        errorMessage_.clear();
        needsFullRepaint_ = true;
        requestUpdate();
      } else {
        if (enteredPin_ == draftPin_) {
          PinStorage::setPin(enteredPin_.c_str());
          result.isCancelled = false;
          activityManager.popActivity();
        } else {
          errorMessage_ = tr(STR_PINS_DO_NOT_MATCH);
          errorUntilMs_ = millis() + 1500;
          enteredPin_.clear();
          draftPin_.clear();
          step_ = 1;
          needsFullRepaint_ = true;
          requestUpdate();
        }
      }
      break;

    case Mode::VerifyForDisable:
      if (PinStorage::verifyPin(enteredPin_.c_str())) {
        PinStorage::clearPin();
        result.isCancelled = false;
        activityManager.popActivity();
      } else {
        errorMessage_ = tr(STR_INCORRECT_PIN);
        errorUntilMs_ = millis() + 1500;
        enteredPin_.clear();
        needsFullRepaint_ = true;
        requestUpdate();
      }
      break;
  }
}

void PinLockActivity::onUnlockSuccess() {
  LockManager::getInstance().unlock();
  if (resumeTarget_ == ResumeTarget::Reader && !resumeBookPath_.empty()) {
    activityManager.goToReader(resumeBookPath_, false, true);
  } else if (resumeTarget_ == ResumeTarget::Home) {
    activityManager.goHome();
  } else {
    activityManager.popActivity();
  }
}

void PinLockActivity::onForgotPin() {
  if (!PinStorage::hasSecurityQuestion()) {
    errorMessage_ = tr(STR_NO_SECURITY_QUESTION);
    errorUntilMs_ = millis() + 2500;
    needsFullRepaint_ = true;
    requestUpdate();
    return;
  }

  const uint8_t qIdx = PinStorage::getSecurityQuestion();
  std::string qTitle;
  if (qIdx == 0) {
    qTitle = tr(STR_QUESTION_GREATEST_LOVE);
  } else if (qIdx == 1) {
    qTitle = tr(STR_QUESTION_FIRST_PET);
  } else {
    qTitle = tr(STR_QUESTION_FAVORITE_COUNTRY);
  }

  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, qTitle, "", 40, InputType::Text, 1),
      [this](const ActivityResult& actRes) {
        if (actRes.isCancelled) {
          needsFullRepaint_ = true;
          requestUpdate();
          return;
        }
        const auto* kb = std::get_if<KeyboardResult>(&actRes.data);
        if (kb && !kb->text.empty()) {
          if (PinStorage::verifySecurityAnswer(kb->text.c_str())) {
            // Opción A: Desbloquea y desactiva el PIN
            PinStorage::clearPin();
            onUnlockSuccess();
          } else {
            errorMessage_ = tr(STR_INCORRECT_ANSWER);
            errorUntilMs_ = millis() + 2500;
            needsFullRepaint_ = true;
            requestUpdate();
          }
        }
      });
}

void PinLockActivity::renderDotsOnly() {
  RenderLock lock;
  renderer.fillRect(dotsX_ + 12, dotsY_ + 2, dotsW_ - 24, dotsH_ - 4, false);

  constexpr size_t dotCount = 6;
  constexpr int dotRadius = 7;
  constexpr int dotSpacing = 28;
  constexpr int dotsTotalW = (dotCount - 1) * dotSpacing;
  const int dotsStartX = dotsX_ + (dotsW_ - dotsTotalW) / 2;
  const int dotsCenterY = dotsY_ + dotsH_ / 2;

  for (size_t i = 0; i < dotCount; ++i) {
    const int cx = dotsStartX + static_cast<int>(i) * dotSpacing;
    if (i < enteredPin_.size()) {
      renderer.fillRoundedRect(cx - dotRadius, dotsCenterY - dotRadius, dotRadius * 2, dotRadius * 2, dotRadius, Color::Black);
    } else {
      renderer.drawRoundedRect(cx - dotRadius, dotsCenterY - dotRadius, dotRadius * 2, dotRadius * 2, 1, dotRadius, true);
    }
  }

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}

void PinLockActivity::render(RenderLock&&) {
  setupLayout();

  renderer.clearScreen();

  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();

  const char* title = "XTEINK X4 PRO";
  const int titleW = renderer.getTextWidth(UI_12_FONT_ID, title, EpdFontFamily::BOLD);
  const int titleY = (screenH > 600) ? 100 : 25;
  renderer.drawText(UI_12_FONT_ID, (screenW - titleW) / 2, titleY, title, true, EpdFontFamily::BOLD);

  const int iconY = titleY + 35;
  freeink::ui::GfxRendererTarget target(renderer);
  target.bitmap(freeink::ui::Rect{static_cast<int16_t>((screenW - icon_tabler_lock_28.w) / 2),
                                  static_cast<int16_t>(iconY),
                                  static_cast<int16_t>(icon_tabler_lock_28.w),
                                  static_cast<int16_t>(icon_tabler_lock_28.h)},
                freeink::ui::bitmapFromIcon(icon_tabler_lock_28), freeink::ui::BitmapMode::Center,
                freeink::ui::Paint::solid(freeink::ui::Color::Black));

  const std::string prompt = getPromptText();
  const auto promptStyle = !errorMessage_.empty() ? EpdFontFamily::BOLD : EpdFontFamily::REGULAR;
  const int promptW = renderer.getTextWidth(UI_10_FONT_ID, prompt.c_str(), promptStyle);
  const int promptY = iconY + 45;
  renderer.drawText(UI_10_FONT_ID, (screenW - promptW) / 2, promptY, prompt.c_str(), true, promptStyle);

  renderer.drawRoundedRect(dotsX_ + 10, dotsY_, dotsW_ - 20, dotsH_, 1, 6, true);

  constexpr size_t dotCount = 6;
  constexpr int dotRadius = 7;
  constexpr int dotSpacing = 28;
  constexpr int dotsTotalW = (dotCount - 1) * dotSpacing;
  const int dotsStartX = dotsX_ + (dotsW_ - dotsTotalW) / 2;
  const int dotsCenterY = dotsY_ + dotsH_ / 2;

  for (size_t i = 0; i < dotCount; ++i) {
    const int cx = dotsStartX + static_cast<int>(i) * dotSpacing;
    if (i < enteredPin_.size()) {
      renderer.fillRoundedRect(cx - dotRadius, dotsCenterY - dotRadius, dotRadius * 2, dotRadius * 2, dotRadius, Color::Black);
    } else {
      renderer.drawRoundedRect(cx - dotRadius, dotsCenterY - dotRadius, dotRadius * 2, dotRadius * 2, 1, dotRadius, true);
    }
  }

  for (const auto& k : keys_) {
    renderer.fillRoundedRect(k.x, k.y, k.w, k.h, 8, Color::White);
    renderer.drawRoundedRect(k.x, k.y, k.w, k.h, 1, 8, true);

    char label[8];
    if (k.key >= '0' && k.key <= '9') {
      snprintf(label, sizeof(label), "%c", k.key);
      const int lw = renderer.getTextWidth(UI_12_FONT_ID, label, EpdFontFamily::BOLD);
      const int lh = renderer.getLineHeight(UI_12_FONT_ID);
      renderer.drawText(UI_12_FONT_ID, k.x + (k.w - lw) / 2, k.y + (k.h - lh) / 2, label, true, EpdFontFamily::BOLD);
    } else if (k.key == 'C') {
      const char* clr = "CLEAR";
      const int lw = renderer.getTextWidth(UI_10_FONT_ID, clr, EpdFontFamily::BOLD);
      const int lh = renderer.getLineHeight(UI_10_FONT_ID);
      renderer.drawText(UI_10_FONT_ID, k.x + (k.w - lw) / 2, k.y + (k.h - lh) / 2, clr, true, EpdFontFamily::BOLD);
    } else if (k.key == 'E') {
      const char* ok = "OK";
      const int lw = renderer.getTextWidth(UI_10_FONT_ID, ok, EpdFontFamily::BOLD);
      const int lh = renderer.getLineHeight(UI_10_FONT_ID);
      renderer.drawText(UI_10_FONT_ID, k.x + (k.w - lw) / 2, k.y + (k.h - lh) / 2, ok, true, EpdFontFamily::BOLD);
    } else if (k.key == 'X') {
      const char* cancel = "CANCEL";
      const int lw = renderer.getTextWidth(UI_10_FONT_ID, cancel, EpdFontFamily::BOLD);
      const int lh = renderer.getLineHeight(UI_10_FONT_ID);
      renderer.drawText(UI_10_FONT_ID, k.x + (k.w - lw) / 2, k.y + (k.h - lh) / 2, cancel, true, EpdFontFamily::BOLD);
    } else if (k.key == 'F') {
      const std::string forgot = tr(STR_FORGOT_PIN);
      const int lw = renderer.getTextWidth(UI_10_FONT_ID, forgot.c_str(), EpdFontFamily::REGULAR);
      const int lh = renderer.getLineHeight(UI_10_FONT_ID);
      renderer.drawText(UI_10_FONT_ID, k.x + (k.w - lw) / 2, k.y + (k.h - lh) / 2, forgot.c_str(), true, EpdFontFamily::REGULAR);
    }
  }

  renderer.displayBuffer(HalDisplay::FULL_REFRESH);
  needsFullRepaint_ = false;
}

void PinLockActivity::loop() {
  if (errorUntilMs_ > 0 && millis() >= errorUntilMs_) {
    errorUntilMs_ = 0;
    errorMessage_.clear();
    needsFullRepaint_ = true;
    requestUpdate();
  }

  int tx = 0;
  int ty = 0;
  if (mappedInput.wasScreenTapped(tx, ty)) {
    for (const auto& k : keys_) {
      if (tx >= k.x && tx <= (k.x + k.w) && ty >= k.y && ty <= (k.y + k.h)) {
        handleKeyPress(k.key);
        return;
      }
    }
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    if (!enteredPin_.empty()) {
      handleKeyPress('C');
    } else if (mode_ != Mode::Unlock) {
      handleKeyPress('X');
    }
  } else if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    handleKeyPress('E');
  }
}
