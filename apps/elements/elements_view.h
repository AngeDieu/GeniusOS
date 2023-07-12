#ifndef ELEMENTS_ELEMENTS_VIEW_H
#define ELEMENTS_ELEMENTS_VIEW_H

#include "elements_view_data_source.h"
#include "single_element_view.h"
#include <escher/message_text_view.h>
#include <escher/view.h>

namespace Elements {

class ElementsView : public Escher::View {
public:
  ElementsView();

  // Escher::View
  void drawRect(KDContext * ctx, KDRect rect) const override;

  void cursorMoved();
  void reload() { markRectAsDirty(bounds()); }
  void dirtyBackground() { m_redrawBackground = true; }

private:
  constexpr static KDCoordinate k_tableTopMargin = 22;
  constexpr static KDCoordinate k_tableLeftMargin = 7;
  constexpr static KDCoordinate k_cellSize = 16;
  constexpr static KDCoordinate k_cellMargin = 1;
  constexpr static KDCoordinate k_zoomedViewMargin = 11;
  constexpr static KDCoordinate k_lanthanideTopMargin = 5;
  constexpr static KDColor k_backgroundColor = KDColorWhite;
  constexpr static KDColor k_ligatureColor = Escher::Palette::GrayMiddle;

  /* rectForCell returns the rect for the colored part of the cell, without
   * its border. This avoid redrawing cells just because their border overlaps
   * a dirty cell. */
  static KDRect RectForCell(uint8_t cellIndex);
  static KDRect RectWithMargins(KDRect rect) { return KDRect(rect.x() - k_cellMargin, rect.y() - k_cellMargin, rect.width() + 2 * k_cellMargin, rect.height() + 2 * k_cellMargin); }
  KDRect SingleElementViewFrame() const;

  // Escher::View
  int numberOfSubviews() const override { return 2; }
  Escher::View * subviewAtIndex(int i) override;
  void layoutSubviews(bool force = false) override;

  void drawElementCell(AtomicNumber z, KDRect cell, KDContext * ctx, KDRect rect) const;
  void drawElementBorder(AtomicNumber z, KDColor color, KDContext * ctx, KDRect rect) const;
  void drawLigatures(KDContext * ctx) const;
  void dirtyElement(AtomicNumber z);

  SingleElementView m_singleElementView;
  Escher::MessageTextView m_nameView;
  mutable bool m_redrawBackground;
};

}

#endif
