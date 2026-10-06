#include "Lyra3CoversTheme.h"

#include <Epub.h>
#include <FsHelpers.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <Xtc.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "CrossPointSettings.h"
#include "RecentBooksStore.h"
#include "activities/boot_sleep/SleepCoverAssets.h"
#include "activities/home/RecentBookProgress.h"
#include "activities/reader/BookReadingStats.h"
#include "activities/reader/ReadingStatsUtils.h"
#include "components/TouchRegistry.h"
#include "components/UITheme.h"
#include "components/UiAppHelpers.h"
#include "fontIds.h"

namespace {
constexpr int cornerRadius = 6;

struct BookSlotLayout {
  int slotX;
  int slotY;
  int slotW;
  int slotH;
  int coverX;
  int coverY;
  int coverW;
  int coverH;
};

BookSlotLayout calculateSlot(int index, int totalBooks, Rect rect) {
  BookSlotLayout l{};
  const int count = std::min(totalBooks, Lyra3CoversMetrics::values.homeRecentBooksCount);
  if (count <= 2) {
    // 1 or 2 large books
    if (count == 1) {
      l.slotW = 200;
      l.slotH = 250;
      l.slotX = rect.x + (rect.width - l.slotW) / 2;
      l.slotY = rect.y + 4;
      l.coverW = 136;
      l.coverH = 186;
      l.coverX = l.slotX + (l.slotW - l.coverW) / 2;
      l.coverY = l.slotY + 4;
    } else {
      // 2 books side by side
      const int sidePad = 24;
      const int colGap = 16;
      l.slotW = (rect.width - 2 * sidePad - colGap) / 2;
      l.slotH = 250;
      l.slotX = rect.x + sidePad + index * (l.slotW + colGap);
      l.slotY = rect.y + 4;
      l.coverW = 126;
      l.coverH = 180;
      l.coverX = l.slotX + (l.slotW - l.coverW) / 2;
      l.coverY = l.slotY + 4;
    }
  } else {
    // 3 to 6 books: 3 per row, up to 2 rows
    const int sidePad = 16;
    const int colGap = 10;
    const int rowGap = 8;
    const int col = index % 3;
    const int row = index / 3;
    l.slotW = (rect.width - 2 * sidePad - 2 * colGap) / 3;
    l.slotH = 194;
    l.slotX = rect.x + sidePad + col * (l.slotW + colGap);
    l.slotY = rect.y + 2 + row * (l.slotH + rowGap);
    l.coverW = 94;
    l.coverH = 134;
    l.coverX = l.slotX + (l.slotW - l.coverW) / 2;
    l.coverY = l.slotY + 4;
  }
  return l;
}

std::string formatBookProgressInfo(const RecentBook& book) {
  std::string info;
  const float percent = RecentBookProgress::loadPercent(book);
  const BookReadingStats stats = RecentBookProgress::loadStats(book);
  if (RecentBookProgress::hasPercent(percent)) {
    info = RecentBookProgress::formatPercent(percent);
  }
  if (stats.totalReadingSeconds > 0) {
    char durBuf[32];
    formatCompactReadingDuration(stats.totalReadingSeconds, durBuf, sizeof(durBuf));
    if (!info.empty()) {
      info += " • ";
    }
    info += durBuf;
  }
  if (info.empty()) {
    info = "0%";
  }
  return info;
}
std::string resolveCoverPath(const RecentBook& book, const BookSlotLayout& slot, GfxRenderer& renderer) {
  // 1. Try exact slot dimensions
  if (!book.coverBmpPath.empty()) {
    const std::string exactPath = UITheme::getCoverThumbPath(book.coverBmpPath, slot.coverW, slot.coverH);
    if (!exactPath.empty() && Storage.exists(exactPath.c_str())) return exactPath;

    // 2. Try slot height (3:5 ratio)
    const std::string slotHPath = UITheme::getCoverThumbPath(book.coverBmpPath, slot.coverH);
    if (!slotHPath.empty() && Storage.exists(slotHPath.c_str())) return slotHPath;

    // 3. Try standard Home cover height (226px)
    const std::string homeHPath = UITheme::getCoverThumbPath(book.coverBmpPath, LyraMetrics::values.homeCoverHeight);
    if (!homeHPath.empty() && Storage.exists(homeHPath.c_str())) return homeHPath;

    // 4. Try grid cover thumbnail (123x180)
    const std::string gridPath = UITheme::getCoverThumbPath(book.coverBmpPath, 123, 180);
    if (!gridPath.empty() && Storage.exists(gridPath.c_str())) return gridPath;

    // 5. If coverBmpPath itself exists as a concrete file
    if (Storage.exists(book.coverBmpPath.c_str())) return book.coverBmpPath;
  }

  // 6. Check book cache for EPUB or XTC
  if (FsHelpers::hasEpubExtension(book.path)) {
    const Epub epub(book.path, "/.crosspoint");
    const std::string baseThumb = epub.getThumbBmpPath();
    if (!baseThumb.empty() && Storage.exists(baseThumb.c_str())) return baseThumb;

    const std::string croppedCover = epub.getCoverBmpPath(true);
    if (!croppedCover.empty() && Storage.exists(croppedCover.c_str())) return croppedCover;

    const std::string uncroppedCover = epub.getCoverBmpPath(false);
    if (!uncroppedCover.empty() && Storage.exists(uncroppedCover.c_str())) return uncroppedCover;

    // If none exists on SD card, try generating on-demand
    if (epub.generateThumbBmp(slot.coverW, slot.coverH, &renderer, SETTINGS.getReaderFontId())) {
      const std::string generated = UITheme::getCoverThumbPath(epub.getThumbBmpPath(), slot.coverW, slot.coverH);
      if (!generated.empty() && Storage.exists(generated.c_str())) return generated;
    }
  } else if (FsHelpers::hasXtcExtension(book.path)) {
    const Xtc xtc(book.path, "/.crosspoint");
    const std::string xtcThumb = xtc.getThumbBmpPath();
    if (!xtcThumb.empty() && Storage.exists(xtcThumb.c_str())) return xtcThumb;
    const std::string xtcCover = xtc.getCoverBmpPath();
    if (!xtcCover.empty() && Storage.exists(xtcCover.c_str())) return xtcCover;
  }

  return "";
}
}  // namespace

void Lyra3CoversTheme::drawRecentBookCover(GfxRenderer& renderer, Rect rect, const std::vector<RecentBook>& recentBooks,
                                           int selectorIndex, bool& coverRendered, bool& coverBufferStored,
                                           bool& bufferRestored, const std::function<bool()>& storeCoverBuffer,
                                           const BookReadingStats* /*stats*/, float /*progressPercent*/,
                                           const GlobalReadingStats* /*globalStats*/,
                                           const char* /*currentChapterTitle*/) const {
  const int count = std::min(static_cast<int>(recentBooks.size()), Lyra3CoversMetrics::values.homeRecentBooksCount);
  if (count <= 0) {
    drawEmptyRecents(renderer, rect);
    return;
  }

  // Draw covers on first render into the framebuffer
  if (!coverRendered) {
    for (int i = 0; i < count; ++i) {
      const BookSlotLayout slot = calculateSlot(i, count, rect);
      const std::string resolvedCoverBmp = resolveCoverPath(recentBooks[i], slot, renderer);
      bool hasCover = false;

      if (!resolvedCoverBmp.empty()) {
        HalFile file;
        if (Storage.openFileForRead("HOME", resolvedCoverBmp, file)) {
          Bitmap bitmap(file);
          if (bitmap.parseHeaders() == BmpReaderError::Ok && bitmap.getWidth() > 0 && bitmap.getHeight() > 0) {
            const float coverHeight = static_cast<float>(bitmap.getHeight());
            const float coverWidth = static_cast<float>(bitmap.getWidth());
            const float srcRatio = coverWidth / coverHeight;
            const float targetRatio = static_cast<float>(slot.coverW) / static_cast<float>(slot.coverH);

            float cropX = 0.0f;
            float cropY = 0.0f;
            if (srcRatio > targetRatio) {
              cropX = std::max(0.0f, 1.0f - (targetRatio / srcRatio));
            } else if (srcRatio < targetRatio) {
              cropY = std::max(0.0f, 1.0f - (srcRatio / targetRatio));
            }

            renderer.drawBitmap(bitmap, slot.coverX, slot.coverY, slot.coverW, slot.coverH, cropX, cropY);
            hasCover = true;
          }
          file.close();
        }
      }

      renderer.drawRect(slot.coverX, slot.coverY, slot.coverW, slot.coverH, true);

      if (!hasCover) {
        renderer.fillRect(slot.coverX, slot.coverY + (slot.coverH / 3), slot.coverW, 2 * slot.coverH / 3, true);
        drawLucideIcon(renderer, icon_book_open_32, slot.coverX + (slot.coverW - 32) / 2, slot.coverY + 24);
      }
    }

    coverBufferStored = storeCoverBuffer();
    coverRendered = coverBufferStored;
  }

  // Draw metadata and selection on top of buffer
  for (int i = 0; i < count; ++i) {
    const BookSlotLayout slot = calculateSlot(i, count, rect);
    const bool bookSelected = (selectorIndex == i);

    const int maxLineWidth = slot.slotW - 6;
    const int maxTitleLines = (count <= 2) ? 2 : 1;
    const auto titleLines = renderer.wrappedText(SMALL_FONT_ID, recentBooks[i].title.c_str(), maxLineWidth, maxTitleLines);
    const int titleLineHeight = renderer.getLineHeight(SMALL_FONT_ID);

    const std::string progressInfo = formatBookProgressInfo(recentBooks[i]);
    const int progressLineHeight = renderer.getLineHeight(SMALL_FONT_ID);

    const int titleBlockHeight = static_cast<int>(titleLines.size()) * titleLineHeight;
    const int textBlockHeight = titleBlockHeight + 2 + progressLineHeight + 4;
    const int dynamicCardHeight = (slot.coverY - slot.slotY) + slot.coverH + 4 + textBlockHeight;

    TouchRegistry::getInstance().add(Rect{slot.slotX, slot.slotY, slot.slotW, dynamicCardHeight}, i, TouchRegistry::Cover);

    if (bookSelected) {
      // Selection highlight wrapping around the cover and filling below
      const int topHeight = slot.coverY - slot.slotY;
      if (topHeight > 0) {
        renderer.fillRoundedRect(slot.slotX, slot.slotY, slot.slotW, topHeight + 2, cornerRadius, true, true, false, false,
                                 Color::LightGray);
      }
      if (slot.coverX > slot.slotX) {
        renderer.fillRectDither(slot.slotX, slot.coverY, slot.coverX - slot.slotX, slot.coverH, Color::LightGray);
      }
      const int rightSideW = (slot.slotX + slot.slotW) - (slot.coverX + slot.coverW);
      if (rightSideW > 0) {
        renderer.fillRectDither(slot.coverX + slot.coverW, slot.coverY, rightSideW, slot.coverH, Color::LightGray);
      }
      const int bottomBoxY = slot.coverY + slot.coverH;
      const int bottomBoxH = dynamicCardHeight - (bottomBoxY - slot.slotY);
      renderer.fillRoundedRect(slot.slotX, bottomBoxY, slot.slotW, bottomBoxH, cornerRadius, false, false, true, true,
                               Color::LightGray);
    }

    // Draw Title below cover
    int textY = slot.coverY + slot.coverH + 4;
    for (const auto& line : titleLines) {
      const int lineW = renderer.getTextWidth(SMALL_FONT_ID, line.c_str());
      const int lineX = slot.slotX + (slot.slotW - lineW) / 2;
      renderer.drawText(SMALL_FONT_ID, lineX, textY, line.c_str(), true);
      textY += titleLineHeight;
    }

    // Draw Progress and Time below title
    textY += 2;
    const int progW = renderer.getTextWidth(SMALL_FONT_ID, progressInfo.c_str());
    const int progX = slot.slotX + (slot.slotW - progW) / 2;
    renderer.drawText(SMALL_FONT_ID, progX, textY, progressInfo.c_str(), true);
  }
}
