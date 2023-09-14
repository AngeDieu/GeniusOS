#include "sum_graph_controller.h"

#include <assert.h>
#include <escher/clipboard.h>
#include <poincare/condensed_sum_layout.h>
#include <poincare/layout_helper.h>
#include <poincare/print.h>
#include <stdlib.h>

#include <cmath>

#include "function_app.h"
#include "poincare_helpers.h"

using namespace Poincare;
using namespace Escher;

namespace Shared {

SumGraphController::SumGraphController(Responder *parentResponder,
                                       FunctionGraphView *graphView,
                                       InteractiveCurveViewRange *range,
                                       CurveViewCursor *cursor)
    : SimpleInteractiveCurveViewController(parentResponder, cursor),
      m_step(Step::FirstParameter),
      m_graphRange(range),
      m_graphView(graphView),
      m_legendView(this) {}

void SumGraphController::viewWillAppear() {
  panToMakeCursorVisible();
  m_graphView->setBannerView(&m_legendView);
  m_graphView->setCursorView(&m_cursorView);
  m_graphView->setFocus(true);
  m_graphView->setAreaHighlightColor(false);
  m_graphView->setAreaHighlight(NAN, NAN);
  m_step = Step::FirstParameter;
  makeCursorVisibleAndReloadBanner();
  SimpleInteractiveCurveViewController::viewWillAppear();
}

void SumGraphController::didBecomeFirstResponder() {
  /* Do not set the textField as first responder when displaying the result
   * so that Copy and Sto apply on the result. */
  if (m_step != Step::Result) {
    App::app()->setFirstResponder(m_legendView.textField());
  }
}

bool SumGraphController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Back && m_step != Step::FirstParameter) {
    m_step = (Step)((int)m_step - 1);
    if (m_step == Step::SecondParameter) {
      App::app()->setFirstResponder(m_legendView.textField());
      m_graphView->setAreaHighlightColor(false);
      m_graphView->setCursorView(&m_cursorView);
    }
    if (m_step == Step::FirstParameter) {
      m_graphView->setAreaHighlight(NAN, NAN);
      moveCursorHorizontallyToPosition(m_startSum);
    }
    reloadBannerView();
    return true;
  }
  if ((event == Ion::Events::Copy || event == Ion::Events::Cut ||
       event == Ion::Events::Sto || event == Ion::Events::Var) &&
      m_step == Step::Result) {
    /* We want to save more digits than we have in the banner to we need to
     * convert the result here */
    constexpr static int precision =
        Preferences::DefaultNumberOfPrintedSignificantDigits;
    constexpr static int bufferSize =
        PrintFloat::charSizeForFloatsWithPrecision(precision);
    char buffer[bufferSize];
    PoincareHelpers::ConvertFloatToText<double>(m_result, buffer, bufferSize,
                                                precision);
    if (event == Ion::Events::Sto || event == Ion::Events::Var) {
      App::app()->storeValue(buffer);
    } else {
      Escher::Clipboard::SharedClipboard()->store(buffer);
    }
    return true;
  }
  return SimpleInteractiveCurveViewController::handleEvent(event);
}

bool SumGraphController::moveCursorHorizontallyToPosition(double x) {
  if (std::isnan(x)) {
    return true;
  }
  /* m_cursorView is a vertical bar so no need to compute its ordinate.
   *
   * TODO We would like to assert that the function is not a parametered
   * function, so we can indeed evaluate the function for parameter x. */
  m_cursor->moveTo(x, x, 0);
  if (m_step == Step::SecondParameter) {
    assert(allowEndLowerThanStart() || m_cursor->x() >= m_startSum);
    m_graphView->setAreaHighlight(std::min(m_startSum, m_cursor->x()),
                                  std::max(m_startSum, m_cursor->x()));
  }
  m_legendView.setEditableZone(m_cursor->x());
  makeCursorVisibleAndReloadBanner();
  m_graphView->reload(true);
  return true;
}

void SumGraphController::makeCursorVisibleAndReloadBanner() {
  makeCursorVisible();
  reloadBannerView();
}

void SumGraphController::makeCursorVisible() {
  float position = m_cursor->x();
  if (!std::isfinite(position)) {
    return;
  }
  ExpiringPointer<Function> function =
      FunctionApp::app()->functionStore()->modelForRecord(selectedRecord());
  float y =
      function
          ->evaluateXYAtParameter(position, FunctionApp::app()->localContext())
          .y();
  // Do not zoom out if user is selecting first parameter
  makeDotVisible(position, y, m_step != Step::FirstParameter);
  makeCursorVisibleOnSecondCurve(position);
}

void SumGraphController::makeCursorVisibleOnSecondCurve(float x) {
  // zoomOut is always true so that the user can see both dots
  makeDotVisible(x, 0.0, true);
}

void SumGraphController::makeDotVisible(float x, float y, bool zoomOut) {
  if (zoomOut) {
    m_graphRange->zoomOutToMakePointVisible(
        x, y, cursorTopMarginRatio(), cursorRightMarginRatio(),
        cursorBottomMarginRatio(), cursorLeftMarginRatio());
  } else {
    m_graphRange->panToMakePointVisible(
        x, y, cursorTopMarginRatio(), cursorRightMarginRatio(),
        cursorBottomMarginRatio(), cursorLeftMarginRatio(),
        curveView()->pixelWidth());
  }
}

void SumGraphController::setRecord(Ion::Storage::Record record) {
  m_graphView->selectRecord(record);
}

bool SumGraphController::textFieldDidFinishEditing(AbstractTextField *textField,
                                                   Ion::Events::Event event) {
  double floatBody =
      ParseInputtedFloatValue<double>(textField->draftTextBuffer());
  if (HasUndefinedValue(floatBody)) {
    return false;
  }
  if ((!allowEndLowerThanStart() && m_step == Step::SecondParameter &&
       floatBody < m_startSum) ||
      !moveCursorHorizontallyToPosition(floatBody)) {
    App::app()->displayWarning(I18n::Message::ForbiddenValue);
    return false;
  }
  return handleEnter();
}

bool SumGraphController::handleLeftRightEvent(Ion::Events::Event event) {
  if (m_step == Step::Result) {
    return false;
  }
  const double oldPosition = m_cursor->x();
  double newPosition = cursorNextStep(oldPosition, OMG::Direction(event));
  if (!allowEndLowerThanStart() && m_step == Step::SecondParameter &&
      newPosition < m_startSum) {
    newPosition = m_startSum;
  }
  return moveCursorHorizontallyToPosition(newPosition);
}

bool SumGraphController::handleEnter() {
  if (m_step == Step::Result) {
    StackViewController *stack = (StackViewController *)parentResponder();
    stack->pop();
  } else {
    Step currentStep = m_step;
    /* Set the step now so that setFirstResponder know it does not need to set
     * the textField as first responder. */
    m_step = (Step)((int)m_step + 1);
    if (currentStep == Step::FirstParameter) {
      m_startSum = m_cursor->x();
      m_graphView->setAreaHighlight(m_startSum, m_startSum);
    } else {
      m_graphView->setAreaHighlightColor(true);
      m_graphView->setCursorView(nullptr);
      App::app()->setFirstResponder(this);
    }
    reloadBannerView();
  }
  return true;
}

void SumGraphController::reloadBannerView() {
  m_legendView.setLegendMessage(legendMessageAtStep(m_step), m_step);
  double endSum = NAN;
  double result;
  Layout functionLayout;
  if (m_step == Step::Result) {
    endSum = m_cursor->x();
    assert(!selectedRecord().isNull());
    Poincare::Context *context = FunctionApp::app()->localContext();
    Poincare::Expression sum = createSumExpression(m_startSum, endSum, context);
    result = PoincareHelpers::ApproximateToScalar<double>(sum, context);
    functionLayout = createFunctionLayout();
  } else {
    m_legendView.setEditableZone(m_cursor->x());
    result = NAN;
  }
  m_result = result;
  m_legendView.setSumLayout(m_step, m_startSum, endSum, result, functionLayout,
                            sumSymbol());
}

Poincare::Expression SumGraphController::createSumExpression(
    double startSum, double endSum, Poincare::Context *context) {
  ExpiringPointer<Function> function =
      FunctionApp::app()->functionStore()->modelForRecord(selectedRecord());
  return function->sumBetweenBounds(startSum, endSum, context);
}

/* Legend View */

SumGraphController::LegendView::LegendView(SumGraphController *controller)
    : m_sum(k_glyphsFormat),
      m_legend(I18n::Message::Default, k_glyphsFormat),
      m_editableZone(controller, m_textBuffer, k_editableZoneBufferSize,
                     controller, k_glyphsFormat) {
  m_textBuffer[0] = 0;
}

void SumGraphController::LegendView::drawRect(KDContext *ctx,
                                              KDRect rect) const {
  ctx->fillRect(bounds(), Palette::GrayMiddle);
}

KDSize SumGraphController::LegendView::minimalSizeForOptimalDisplay() const {
  return KDSize(0, k_legendHeight);
}

void SumGraphController::LegendView::setLegendMessage(I18n::Message message,
                                                      Step step) {
  m_legend.setMessage(message);
  layoutSubviews(step, false);
}

void SumGraphController::LegendView::setEditableZone(double d) {
  char buffer[k_valuesBufferSize];
  PoincareHelpers::ConvertFloatToTextWithDisplayMode<double>(
      d, buffer, k_valuesBufferSize, k_valuesPrecision,
      Preferences::PrintFloatMode::Decimal);
  m_editableZone.setText(buffer);
}

void SumGraphController::LegendView::setSumLayout(Step step, double start,
                                                  double end, double result,
                                                  Layout functionLayout,
                                                  CodePoint sumSymbol) {
  Layout sumLayout = CodePointLayout::Builder(sumSymbol);
  if (step != Step::FirstParameter) {
    static_assert(k_valuesBufferSize <= k_editableZoneBufferSize);
    char buffer[k_editableZoneBufferSize];
    Layout endLayout;
    if (step == Step::SecondParameter) {
      endLayout = HorizontalLayout::Builder();
      static_cast<HorizontalLayout &>(endLayout).setEmptyVisibility(
          EmptyRectangle::State::Hidden);
    } else {
      PoincareHelpers::ConvertFloatToTextWithDisplayMode<double>(
          end, buffer, k_valuesBufferSize, k_valuesPrecision,
          Preferences::PrintFloatMode::Decimal);
      endLayout = LayoutHelper::String(buffer, strlen(buffer));
    }
    PoincareHelpers::ConvertFloatToTextWithDisplayMode<double>(
        start, buffer, k_valuesBufferSize, k_valuesPrecision,
        Preferences::PrintFloatMode::Decimal);
    sumLayout = CondensedSumLayout::Builder(
        sumLayout, LayoutHelper::String(buffer, strlen(buffer)), endLayout);
    if (step == Step::Result) {
      int resultPrecision =
          Poincare::Preferences::sharedPreferences->numberOfSignificantDigits();
      PoincareHelpers::ConvertFloatToText<double>(
          result, buffer, k_editableZoneBufferSize, resultPrecision);
      if (functionLayout.isUninitialized()) {
        /* If function is uninitialized, display "Area = "
         * This case should not occur with a sum of terms of a sequence
         * If it does, "Area = " should be replaced with "Sum = " */
        assert(sumSymbol == UCodePointIntegral);
        sumLayout = defaultSumResultLayout(buffer);
      } else {
        sumLayout = HorizontalLayout::Builder(
            sumLayout, functionLayout, LayoutHelper::String(" = ", 3),
            LayoutHelper::String(buffer, strlen(buffer)));
        if (sumLayout.layoutSize(KDFont::Size::Small).width() >
            bounds().width()) {
          /* If layout is too large, display "Area = "
           * This case should not occur with a sum of terms of a sequence
           * If it does, "Area = " should be replaced with "Sum = " */
          assert(sumSymbol == UCodePointIntegral);
          sumLayout = defaultSumResultLayout(buffer);
        }
      }
    }
  }
  m_sum.setLayout(sumLayout);
  m_sum.setAlignment(
      step == Step::Result ? KDGlyph::k_alignCenter : KDGlyph::k_alignLeft,
      KDGlyph::k_alignCenter);
  layoutSubviews(step, false);
}

View *SumGraphController::LegendView::subviewAtIndex(int index) {
  assert(index >= 0 && index < 3);
  if (index == 0) {
    return &m_sum;
  }
  if (index == 1) {
    return &m_editableZone;
  }
  return &m_legend;
}

void SumGraphController::LegendView::layoutSubviews(bool force) {
  layoutSubviews(Step::FirstParameter, force);
}

void SumGraphController::LegendView::layoutSubviews(Step step, bool force) {
  KDCoordinate width = bounds().width();
  KDCoordinate heigth = bounds().height();
  KDSize legendSize = m_legend.minimalSizeForOptimalDisplay();
  constexpr static KDCoordinate horizontalMargin = 7;

  if (legendSize.width() > 0) {
    setChildFrame(&m_sum,
                  KDRect(horizontalMargin, k_symbolHeightMargin,
                         width - legendSize.width(),
                         m_sum.minimalSizeForOptimalDisplay().height()),
                  force);
    setChildFrame(&m_legend,
                  KDRect(width - legendSize.width() - horizontalMargin, 0,
                         legendSize.width(), heigth),
                  force);
  } else {
    setChildFrame(&m_sum, bounds(), force);
    setChildFrame(&m_legend, KDRectZero, force);
  }

  KDRect frame =
      (step == Step::Result)
          ? KDRectZero
          : KDRect(horizontalMargin + KDFont::GlyphWidth(k_font),
                   k_symbolHeightMargin + k_sigmaHeight / 2 -
                       (step == Step::SecondParameter) * editableZoneHeight(),
                   editableZoneWidth(), editableZoneHeight());
  setChildFrame(&m_editableZone, frame, force);
}

Layout SumGraphController::LegendView::defaultSumResultLayout(
    const char *resultBuffer) {
  // Create layout of "Area = [result]"
  const char *areaMessage = I18n::translate(I18n::Message::Area);
  // strlen("Oppervlakte") + strlen(" = ") + 0;
  constexpr static int bufferSize = 11 + 3 + 1;
  char buffer[bufferSize];
  int length = Print::CustomPrintf(buffer, bufferSize, "%s = ", areaMessage);
  assert(length < bufferSize);
  return HorizontalLayout::Builder(
      LayoutHelper::String(buffer, length),
      LayoutHelper::String(resultBuffer, strlen(resultBuffer)));
}

}  // namespace Shared
