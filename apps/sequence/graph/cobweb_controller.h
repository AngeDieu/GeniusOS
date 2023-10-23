#ifndef SEQUENCE_COBWEB_CONTROLLER_H
#define SEQUENCE_COBWEB_CONTROLLER_H

#include <poincare/layout.h>

#include "../../shared/sum_graph_controller.h"
#include "../../shared/xy_banner_view.h"
#include "apps/shared/curve_view_cursor.h"
#include "apps/shared/function_graph_controller.h"
#include "apps/shared/sequence_store.h"
#include "cobweb_graph_view.h"
#include "curve_view_range.h"
#include "graph_view.h"

namespace Sequence {

class GraphController;

class CobwebController : public Shared::SimpleInteractiveCurveViewController,
                         public Shared::FunctionBannerDelegate {
 public:
  CobwebController(Responder* parentResponder,
                   Escher::InputEventHandlerDelegate* inputEventHandlerDelegate,
                   GraphView* graphView, CurveViewRange* graphRange,
                   Shared::CurveViewCursor* cursor,
                   Shared::XYBannerView* bannerView,
                   Shared::CursorView* cursorView,
                   Shared::SequenceStore* sequenceStore);
  const char* title() override;
  TELEMETRY_ID("Cobweb");
  void viewWillAppear() override;
  void setRecord(Ion::Storage::Record record);
  bool isRecordSuitable() const;

  bool stepIsInitialized() const { return m_step >= 0; }
  void resetStep() { m_step = -1; }
  int rankAtCurrentStep() const {
    assert(m_step >= 0);
    return rankAtStep(m_step);
  }

 private:
  float cursorBottomMarginRatio() const override {
    return cursorBottomMarginRatioForBannerHeight(
        m_bannerView->minimalSizeForOptimalDisplay().height());
  }
  Shared::InteractiveCurveViewRange* interactiveCurveViewRange() override {
    return &m_graphRange;
  }
  Shared::AbstractPlotView* curveView() override { return &m_graphView; }
  Shared::XYBannerView* bannerView() override { return m_bannerView; };
  int rankAtStep(int step) const { return step + sequence()->initialRank(); }
  void reloadBannerView() override;
  bool handleLeftRightEvent(Ion::Events::Event event) override;
  bool handleEnter() override;
  bool handleZoom(Ion::Events::Event event) override;
  void setupRange();
  bool updateStep(int delta);
  Shared::ExpiringPointer<Shared::Sequence> sequence() const;
  CobwebGraphView m_graphView;
  Shared::XYBannerView* m_bannerView;
  Shared::InteractiveCurveViewRange m_graphRange;
  Ion::Storage::Record m_record;

 private:
  void privateModalViewAltersFirstResponder(
      FirstResponderAlteration alteration) override;
  constexpr static float k_margin = 0.12;
  int m_step;
  bool m_initialZoom;
  bool m_isSuitable;
  Shared::SequenceStore* m_sequenceStore;
};

}  // namespace Sequence

#endif
