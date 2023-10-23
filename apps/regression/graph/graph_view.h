#ifndef REGRESSION_GRAPH_VIEW_H
#define REGRESSION_GRAPH_VIEW_H

#include <apps/constant.h>
#include <apps/shared/plot_view_policies.h>

#include "../store.h"

namespace Regression {

class RegressionPlotPolicy : public Shared::PlotPolicy::WithCurves {
 protected:
  void drawPlot(const Shared::AbstractPlotView* plotView, KDContext* ctx,
                KDRect rect) const;

  Store* m_store;
};

class GraphView
    : public Shared::PlotView<
          Shared::PlotPolicy::TwoLabeledAxes, RegressionPlotPolicy,
          Shared::PlotPolicy::WithBanner, Shared::PlotPolicy::WithCursor> {
 public:
  GraphView(Shared::InteractiveCurveViewRange* range, Store* store,
            Shared::CurveViewCursor* cursor, Shared::BannerView* bannerView,
            Shared::CursorView* cursorView);
};

}  // namespace Regression

#endif
