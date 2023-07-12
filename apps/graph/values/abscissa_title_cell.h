#ifndef GRAPH_ABSCISSA_TITLE_CELL_H
#define GRAPH_ABSCISSA_TITLE_CELL_H

#include <escher/even_odd_message_text_cell.h>
#include <apps/shared/separable.h>

namespace Graph {

class AbscissaTitleCell : public Escher::EvenOddMessageTextCell, public Shared::Separable {
public:
  AbscissaTitleCell() : Escher::EvenOddMessageTextCell(), Separable() {}
  void drawRect(KDContext * ctx, KDRect rect) const override;
  void layoutSubviews(bool force = false) override;
private:
  void didSetSeparator() override;
};

}

#endif
