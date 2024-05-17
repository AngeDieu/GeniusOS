#ifndef CALCULATION_VECTOR_GRAPH_CELL_H
#define CALCULATION_VECTOR_GRAPH_CELL_H

#include <apps/shared/plot_view_policies.h>

#include "illustration_cell.h"
#include "vector_model.h"

namespace Calculation {

class VectorGraphPolicy : public Shared::PlotPolicy::WithCurves {
 public:
  constexpr static float k_arcRadiusInPixels = 24;  // Ad hoc

 protected:
  constexpr static float k_arrowSizeInPixels = 8;  // Ad hoc

  void drawPlot(const Shared::AbstractPlotView* plotView, KDContext* ctx,
                KDRect rect) const;

  VectorModel* m_model;
};

class VectorGraphView
    : public Shared::PlotView<Shared::PlotPolicy::TwoLabeledAxes,
                              VectorGraphPolicy, Shared::PlotPolicy::NoBanner,
                              Shared::PlotPolicy::NoCursor> {
 public:
  VectorGraphView(VectorModel* model);
};

class VectorGraphCell : public IllustrationCell {
 public:
  VectorGraphCell(VectorModel* model) : m_view(model) {}
  void reload() override { m_view.reload(); }

 private:
  View* view() override { return &m_view; }
  VectorGraphView m_view;
};

}  // namespace Calculation

#endif
