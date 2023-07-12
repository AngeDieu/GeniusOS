#ifndef GRAPH_GRAPH_CONTROLLER_H
#define GRAPH_GRAPH_CONTROLLER_H

#include "banner_view.h"
#include "curve_parameter_controller.h"
#include "graph_controller_helper.h"
#include "graph_view.h"
#include "points_of_interest_cache.h"
#include <apps/shared/continuous_function_store.h>
#include <apps/shared/with_record.h>
#include <apps/shared/function_graph_controller.h>
#include <apps/shared/curve_view_cursor.h>
#include <apps/shared/round_cursor_view.h>
#include <apps/shared/interactive_curve_view_range.h>

namespace Graph {

class GraphController : public Shared::FunctionGraphController, public GraphControllerHelper {
public:
  GraphController(Escher::Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, Shared::InteractiveCurveViewRange * curveViewRange, Shared::CurveViewCursor * cursor, int * indexFunctionSelectedByCursor, Escher::ButtonRowController * header);

  I18n::Message emptyMessage() override;
  void viewWillAppear() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;

  Poincare::Range2D optimalRange(bool computeX, bool computeY, Poincare::Range2D originalRange) const override;

  bool displayDerivativeInBanner() const;
  PointsOfInterestCache * pointsOfInterestForRecord(Ion::Storage::Record record);
  PointsOfInterestCache * pointsOfInterestForSelectedRecord() {
    return pointsOfInterestForRecord(functionStore()->activeRecordAtIndex(indexFunctionSelectedByCursor()));
  }

private:
  class FunctionSelectionController : public Shared::FunctionGraphController::FunctionSelectionController {
  public:
    FunctionSelectionController(GraphController * graphController) : Shared::FunctionGraphController::FunctionSelectionController(graphController) {}

    CurveSelectionCellWithChevron * reusableCell(int index, int type) override { assert(index >= 0 && index < k_maxNumberOfDisplayableFunctions); return m_cells + index; }
    int reusableCellCount(int type) override { return k_maxNumberOfDisplayableFunctions; }

  private:
    constexpr static int k_maxNumberOfDisplayableFunctions = 7;

    Poincare::Layout nameLayoutAtIndex(int j) const override;

    CurveSelectionCellWithChevron m_cells[k_maxNumberOfDisplayableFunctions];
  };

  // GraphControllerHelper
  BannerView * bannerView() override { return &m_bannerView; }
  GraphView * graphView() override { return &m_view; }
  Shared::FunctionBannerDelegate * functionBannerDelegate() override { return this; }

  void jumpToLeftRightCurve(double t, int direction, int functionsCount, Ion::Storage::Record record) override;

  void reloadBannerView() override;
  bool moveCursorHorizontally(int direction, int scrollSpeed = 1) override;
  int nextCurveIndexVertically(bool goingUp, int currentSelectedCurve, Poincare::Context * context, int currentSubCurveIndex, int * nextSubCurveIndex) const override;
  double defaultCursorT(Ion::Storage::Record record, bool ignoreMargins) override;
  Shared::InteractiveCurveViewRange * interactiveCurveViewRange() override { return m_graphRange; }
  GraphView * functionGraphView() override { return &m_view; }
  CurveParameterController * curveParameterController() override { return &m_curveParameterController; }
  Shared::WithRecord * curveParameterControllerWithRecord() override { return &m_curveParameterController; }
  FunctionSelectionController * curveSelectionController() const override { return const_cast<FunctionSelectionController *>(&m_functionSelectionController); }
  Shared::ContinuousFunctionStore * functionStore() const override { return static_cast<Shared::ContinuousFunctionStore *>(Shared::FunctionGraphController::functionStore()); }
  void moveCursorAndCenterIfNeeded(double t) override;

  bool defaultRangeIsNormalized() const { return functionStore()->displaysFunctionsToNormalize(); }
  void interestingFunctionRange(Shared::ExpiringPointer<Shared::ContinuousFunction> f, float tMin, float tMax, float step, float * xm, float * xM, float * ym, float * yM) const;

  void reloadBannerViewForCursorOnFunction(Shared::CurveViewCursor * cursor, Ion::Storage::Record record, Shared::FunctionStore * functionStore, Poincare::Context * context, bool cappedNumberOfSignificantDigits = false) override;

  Shared::RoundCursorView m_cursorView;
  BannerView m_bannerView;
  GraphView m_view;
  Shared::InteractiveCurveViewRange * m_graphRange;
  CurveParameterController m_curveParameterController;
  FunctionSelectionController m_functionSelectionController;
  constexpr static int k_numberOfCaches = 5;
  Ion::RingBuffer<PointsOfInterestCache, k_numberOfCaches> m_pointsOfInterest;
};

}

#endif
