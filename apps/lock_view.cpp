#include "lock_view.h"

const uint8_t lockMask[LockView::k_lockHeight][LockView::k_lockWidth] = {
    {0xFF, 0xE1, 0x0C, 0x00, 0x0C, 0xE1, 0xFF},
    {0xFF, 0x0C, 0xE1, 0xFF, 0xE1, 0x0C, 0xFF},
    {0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

void LockView::drawRect(KDContext *ctx, KDRect rect) const {
  KDRect frame((bounds().width() - k_lockWidth) / 2,
               (bounds().height() - k_lockHeight) / 2, k_lockWidth,
               k_lockHeight);
  KDColor lockWorkingBuffer[LockView::k_lockHeight * LockView::k_lockWidth];
  ctx->blendRectWithMask(frame, KDColorWhite, (const uint8_t *)lockMask,
                         lockWorkingBuffer);
}

KDSize LockView::minimalSizeForOptimalDisplay() const {
  return KDSize(k_lockWidth, k_lockHeight);
}
