#include "calculation_cell.h"

#include <apps/i18n.h>
#include <assert.h>
#include <escher/metric.h>

#include <algorithm>

using namespace Escher;

namespace Distributions {

CalculationCell::CalculationCell(Responder* parentResponder,
                                 BoxesDelegate* boxesDelegate,
                                 TextFieldDelegate* textFieldDelegate)
    : m_text(I18n::Message::Default,
             {.horizontalAlignment = KDGlyph::k_alignCenter}),
      m_calculation(parentResponder, boxesDelegate, textFieldDelegate),
      m_isResponder(true) {}

Responder* CalculationCell::responder() {
  if (m_isResponder) {
    return &m_calculation;
  }
  return nullptr;
}

void CalculationCell::setResponder(bool shouldBeResponder) {
  m_isResponder = shouldBeResponder;
}

void CalculationCell::setHighlighted(bool highlight) {
  HighlightCell::setHighlighted(highlight);
  m_calculation.setHighlighted(highlight);
}

KDSize CalculationCell::minimalSizeForOptimalDisplay() const {
  KDSize textSize = m_text.minimalSizeForOptimalDisplay();
  return KDSize(2 * k_margin + textSize.width() + calculationCellWidth() +
                    2 * Escher::Metric::CellSeparatorThickness,
                KDFont::GlyphHeight(KDFont::Size::Large));
}

void CalculationCell::drawRect(KDContext* ctx, KDRect rect) const {
  ctx->fillRect(bounds(), KDColorWhite);
  if (m_isResponder) {
    KDSize textSize = m_text.minimalSizeForOptimalDisplay();
    ctx->strokeRect(KDRect(2 * k_margin + textSize.width(), 0,
                           calculationCellWidth() +
                               2 * Escher::Metric::CellSeparatorThickness,
                           bounds().height()),
                    Palette::GrayMiddle);
  }
}

AbstractEditableTextCell* CalculationCell::editableTextCell() {
  return &m_calculation;
}

MessageTextView* CalculationCell::messageTextView() { return &m_text; }

int CalculationCell::numberOfSubviews() const { return 2; }

View* CalculationCell::subviewAtIndex(int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    return &m_text;
  }
  return &m_calculation;
}

void CalculationCell::layoutSubviews(bool force) {
  KDSize textSize = m_text.minimalSizeForOptimalDisplay();
  // A 1px offset is needed to vertically center text on calculation
  setChildFrame(&m_text,
                KDRect(k_margin, 0, textSize.width(),
                       bounds().height() - k_textBottomOffset),
                force);
  setChildFrame(
      &m_calculation,
      KDRect(2 * k_margin + textSize.width() +
                 Escher::Metric::CellSeparatorThickness,
             Escher::Metric::CellSeparatorThickness, calculationCellWidth(),
             bounds().height() - 2 * Escher::Metric::CellSeparatorThickness),
      force);
}

KDCoordinate CalculationCell::calculationCellWidth() const {
  KDCoordinate calculationCellWidth =
      m_calculation.minimalSizeForOptimalDisplay().width();
  KDCoordinate glyphWidth = KDFont::GlyphWidth(KDFont::Size::Large);
  KDCoordinate minTextFieldWidth = 4 * glyphWidth + TextCursorView::k_width;
  KDCoordinate maxTextFieldWidth = 14 * glyphWidth + TextCursorView::k_width;
  return std::min(maxTextFieldWidth,
                  std::max(minTextFieldWidth, calculationCellWidth));
}

}  // namespace Distributions
