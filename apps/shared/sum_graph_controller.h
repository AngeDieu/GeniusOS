#ifndef SHARED_SUM_GRAPH_CONTROLLER_H
#define SHARED_SUM_GRAPH_CONTROLLER_H

#include <escher/expression_view.h>
#include <poincare/print_float.h>
#include "function_graph_view.h"
#include "vertical_cursor_view.h"
#include "simple_interactive_curve_view_controller.h"
#include "function.h"
#include "text_field_delegate.h"
#include "expiring_pointer.h"

namespace Shared {

class SumGraphController : public SimpleInteractiveCurveViewController {
public:
  SumGraphController(Responder * parentResponder, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, FunctionGraphView * curveView, InteractiveCurveViewRange * range, CurveViewCursor * cursor, CodePoint sumSymbol);

  void viewWillAppear() override;
  void didBecomeFirstResponder() override;
  bool handleEvent(Ion::Events::Event event) override;
  bool textFieldDidFinishEditing(Escher::AbstractTextField * textField, const char * text, Ion::Events::Event event) override;

  TELEMETRY_ID("Sum");
  void setRecord(Ion::Storage::Record record);

protected:
  virtual bool moveCursorHorizontallyToPosition(double position);
  void makeCursorVisibleAndReload();
  virtual void makeCursorVisible();
  void makeDotVisible(float x, float y, bool zoomOut);
  Ion::Storage::Record selectedRecord() { return m_graphView->selectedRecord(); }

  enum class Step {
    FirstParameter = 0,
    SecondParameter = 1,
    Result = 2
  };

  Step m_step;
  double m_startSum;
  double m_result;
  InteractiveCurveViewRange * m_graphRange;
  FunctionGraphView * m_graphView;

private:
  float cursorTopMarginRatio() const override { return 0.06f; }
  float cursorBottomMarginRatio() const override { return 0.28f; }
  bool handleLeftRightEvent(Ion::Events::Event event) override;
  bool handleEnter() override;
  void reloadBannerView() override;
  InteractiveCurveViewRange * interactiveCurveViewRange() override { return m_graphRange; }
  AbstractPlotView * curveView() override { return m_graphView; }

  virtual I18n::Message legendMessageAtStep(Step step) = 0;
  virtual double cursorNextStep(double position, int direction) = 0;
  virtual Poincare::Layout createFunctionLayout() = 0;
  virtual Poincare::Expression createSumExpression(double startSum, double endSum, Poincare::Context * context);

  class LegendView : public Escher::View {
  public:
    LegendView(SumGraphController * controller, Escher::InputEventHandlerDelegate * inputEventHandlerDelegate, CodePoint sumSymbol);
    LegendView(const LegendView& other) = delete;
    LegendView(LegendView&& other) = delete;

    LegendView& operator=(const LegendView& other) = delete;
    LegendView& operator=(LegendView&& other) = delete;

    Escher::TextField * textField() { return &m_editableZone; }
    KDSize minimalSizeForOptimalDisplay() const override;
    void drawRect(KDContext * ctx, KDRect rect) const override;
    void setLegendMessage(I18n::Message message, Step step);
    void setEditableZone(double d);
    void setSumLayout(Step step, double start, double end, double result, Poincare::Layout functionLayout);

  private:
    constexpr static size_t k_editableZoneBufferSize = Poincare::PrintFloat::k_maxFloatCharSize;
    constexpr static int k_valuesPrecision = Poincare::Preferences::MediumNumberOfSignificantDigits;
    constexpr static int k_valuesBufferSize = Poincare::PrintFloat::charSizeForFloatsWithPrecision(k_valuesPrecision);
    constexpr static KDCoordinate k_legendHeight = 35;
    constexpr static KDFont::Size k_font = KDFont::Size::Small;
    constexpr static KDCoordinate k_symbolHeightMargin = 8;
    constexpr static KDCoordinate k_sigmaHeight = 18;

    constexpr static KDCoordinate editableZoneWidth() { return 12*KDFont::GlyphWidth(k_font); }
    constexpr static KDCoordinate editableZoneHeight() { return KDFont::GlyphHeight(k_font); }

    int numberOfSubviews() const override { return 3; }
    Escher::View * subviewAtIndex(int index) override;
    void layoutSubviews(bool force = false) override;
    void layoutSubviews(Step step, bool force);
    Poincare::Layout defaultSumResultLayout(const char * resultBuffer);

    Escher::ExpressionView m_sum;
    Escher::MessageTextView m_legend;
    Escher::TextField m_editableZone;
    char m_textBuffer[k_editableZoneBufferSize];
    CodePoint m_sumSymbol;
  };

  LegendView m_legendView;
  VerticalCursorView m_cursorView;
};

}

#endif
