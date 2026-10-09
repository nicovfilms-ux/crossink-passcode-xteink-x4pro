#include "LostAndFoundActivity.h"

#include <FreeInkUIGfxRenderer.h>
#include <FreeInkUIIcon.h>
#include <I18n.h>

#include "CrossPointSettings.h"
#include "components/UITheme.h"
#include "components/icons/keyboardIcons.h"
#include "fontIds.h"

LostAndFoundActivity::LostAndFoundActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
    : Activity("LostAndFound", renderer, mappedInput) {}

void LostAndFoundActivity::onEnter() {
  Activity::onEnter();
  requestUpdate();
}

void LostAndFoundActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const int screenW = renderer.getScreenWidth();
  const int screenH = renderer.getScreenHeight();

  // Title
  const std::string title = tr(STR_LOST_AND_FOUND);
  const int titleW = renderer.getTextWidth(UI_12_FONT_ID, title.c_str(), EpdFontFamily::BOLD);
  const int titleY = (screenH > 600) ? 80 : 20;
  renderer.drawText(UI_12_FONT_ID, (screenW - titleW) / 2, titleY, title.c_str(), true, EpdFontFamily::BOLD);

  // Icon
  const int iconY = titleY + 35;
  freeink::ui::GfxRendererTarget target(renderer);
  target.bitmap(freeink::ui::Rect{static_cast<int16_t>((screenW - icon_info_28.w) / 2), static_cast<int16_t>(iconY),
                                  static_cast<int16_t>(icon_info_28.w), static_cast<int16_t>(icon_info_28.h)},
                freeink::ui::bitmapFromIcon(icon_info_28), freeink::ui::BitmapMode::Center,
                freeink::ui::Paint::solid(freeink::ui::Color::Black));

  // Explanatory message
  const std::string infoText = tr(STR_LOST_AND_FOUND_INFO);
  const int maxTextW = screenW - 60;
  auto infoLines = renderer.wrappedText(UI_10_FONT_ID, infoText.c_str(), maxTextW, 3, EpdFontFamily::REGULAR);
  const int infoLineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  int curInfoY = iconY + 40;
  for (const auto& line : infoLines) {
    const int lw = renderer.getTextWidth(UI_10_FONT_ID, line.c_str(), EpdFontFamily::REGULAR);
    renderer.drawText(UI_10_FONT_ID, (screenW - lw) / 2, curInfoY, line.c_str(), true, EpdFontFamily::REGULAR);
    curInfoY += infoLineHeight;
  }

  // Card box
  const int cardW = screenW - 48;
  const int cardX = 24;
  const int cardY = curInfoY + 20;
  const bool hasContact = (SETTINGS.contactName[0] != '\0' || SETTINGS.contactPhone[0] != '\0');

  int cardContentH = 0;
  if (hasContact) {
    cardContentH = 150;
  } else {
    cardContentH = 90;
  }

  renderer.fillRoundedRect(cardX, cardY, cardW, cardContentH, 8, Color::White);
  renderer.drawRoundedRect(cardX, cardY, cardW, cardContentH, 2, 8, true);

  if (hasContact) {
    int rowY = cardY + 16;

    // Contact name
    const std::string nameLabel = tr(STR_CONTACT_NAME);
    renderer.drawText(SMALL_FONT_ID, cardX + 16, rowY, nameLabel.c_str(), true, EpdFontFamily::BOLD);
    rowY += renderer.getLineHeight(SMALL_FONT_ID) + 2;

    const std::string nameVal = SETTINGS.contactName[0] != '\0' ? SETTINGS.contactName : "--";
    renderer.drawText(UI_12_FONT_ID, cardX + 16, rowY, nameVal.c_str(), true, EpdFontFamily::BOLD);
    rowY += renderer.getLineHeight(UI_12_FONT_ID) + 14;

    // Contact phone
    const std::string phoneLabel = tr(STR_CONTACT_PHONE);
    renderer.drawText(SMALL_FONT_ID, cardX + 16, rowY, phoneLabel.c_str(), true, EpdFontFamily::BOLD);
    rowY += renderer.getLineHeight(SMALL_FONT_ID) + 2;

    const std::string phoneVal = SETTINGS.contactPhone[0] != '\0' ? SETTINGS.contactPhone : "--";
    renderer.drawText(UI_12_FONT_ID, cardX + 16, rowY, phoneVal.c_str(), true, EpdFontFamily::BOLD);
  } else {
    const std::string noContact = tr(STR_NO_CONTACT_INFO);
    auto lines = renderer.wrappedText(SMALL_FONT_ID, noContact.c_str(), cardW - 32, 4, EpdFontFamily::REGULAR);
    int rowY = cardY + 16;
    for (const auto& line : lines) {
      renderer.drawText(SMALL_FONT_ID, cardX + 16, rowY, line.c_str(), true, EpdFontFamily::REGULAR);
      rowY += renderer.getLineHeight(SMALL_FONT_ID);
    }
  }

  // Close button
  closeBtn_.w = 160;
  closeBtn_.h = 44;
  closeBtn_.x = (screenW - closeBtn_.w) / 2;
  closeBtn_.y = (screenH > 600) ? (screenH - 120) : (screenH - 70);

  renderer.fillRoundedRect(closeBtn_.x, closeBtn_.y, closeBtn_.w, closeBtn_.h, 8, Color::White);
  renderer.drawRoundedRect(closeBtn_.x, closeBtn_.y, closeBtn_.w, closeBtn_.h, 2, 8, true);

  const std::string closeText = tr(STR_BACK);
  const int closeW = renderer.getTextWidth(UI_10_FONT_ID, closeText.c_str(), EpdFontFamily::BOLD);
  const int closeH = renderer.getLineHeight(UI_10_FONT_ID);
  renderer.drawText(UI_10_FONT_ID, closeBtn_.x + (closeBtn_.w - closeW) / 2, closeBtn_.y + (closeBtn_.h - closeH) / 2,
                    closeText.c_str(), true, EpdFontFamily::BOLD);

  // Bottom hint
  GUI.drawButtonHints(renderer, tr(STR_BACK), "", "", "");

  renderer.displayBuffer(HalDisplay::FAST_REFRESH);
}

void LostAndFoundActivity::loop() {
  int tx = 0;
  int ty = 0;
  if (mappedInput.wasScreenTapped(tx, ty)) {
    activityManager.popActivity();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Back) ||
      mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    activityManager.popActivity();
  }
}
