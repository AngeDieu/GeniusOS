#include "layout_field.h"

#include <apps/i18n.h>
#include <poincare/code_point_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/symbol.h>

using namespace Poincare;

namespace Calculation {

bool LayoutField::handleEvent(Ion::Events::Event event) {
  if (event != Ion::Events::Division && event.isKeyPress()) {
    m_divisionCycleWithAns = TrinaryBoolean::Unknown;
  }
  if (event == Ion::Events::Back) {
    return false;
  }
  if (isEditing() && isEmpty() &&
      (event == Ion::Events::Multiplication || event == Ion::Events::Plus ||
       event == Ion::Events::Power || event == Ion::Events::Square ||
       event == Ion::Events::Sto)) {
    insertText(Symbol::k_ansAliases.mainAlias());
  }
  if (event == Ion::Events::Minus && isEditing() &&
      fieldContainsSingleMinusSymbol()) {
    setText(Symbol::k_ansAliases.mainAlias());
    // The Minus symbol will be addded by Escher::LayoutField::handleEvent
  }
  if (event == Ion::Events::Division && isEditing()) {
    if (m_divisionCycleWithAns == TrinaryBoolean::Unknown) {
      m_currentStep = DivisionCycleStep::Start;
      m_divisionCycleWithAns =
          isEmpty() ? TrinaryBoolean::True : TrinaryBoolean::False;
    }
    return handleDivision();
  }
  return Escher::LayoutField::handleEvent(event);
}

bool LayoutField::fieldContainsSingleMinusSymbol() const {
  if (layout().isHorizontal() && layout().numberOfChildren() == 1) {
    Layout child = layout().childAtIndex(0);
    if (child.type() == LayoutNode::Type::CodePointLayout) {
      return static_cast<CodePointLayout &>(child).codePoint() == '-';
    }
  }
  return false;
}

bool LayoutField::handleDivision() {
  assert(m_divisionCycleWithAns != TrinaryBoolean::Unknown);
  bool mixedFractionsEnabled =
      Preferences::sharedPreferences->mixedFractionsAreEnabled();
  Ion::Events::Event event = Ion::Events::Division;

  if (m_divisionCycleWithAns == TrinaryBoolean::True) {
    /* When we are in the "Ans" case, the cycle is the following :
     * Start -> DenominatorOfAnsFraction -> NumeratorOfEmptyFraction (->
     * MixedFraction) -> DenominatorOfAnsFraction -> etc with the mixed fraction
     * step only kept when the country enables it */
    clearLayout();
    switch (m_currentStep) {
      case DivisionCycleStep::DenominatorOfAnsFraction:
        // DenominatorOfAnsFraction -> NumeratorOfEmptyFraction
        m_currentStep = DivisionCycleStep::NumeratorOfEmptyFraction;
        break;
      case DivisionCycleStep::NumeratorOfEmptyFraction:
        // NumeratorOfEmptyFraction -> MixedFraction
        m_currentStep = DivisionCycleStep::MixedFraction;
        if (mixedFractionsEnabled) {
          return handleEventWithText(
              I18n::translate(I18n::Message::MixedFractionCommand));
        }
        /* If mixed fractions are not enabled, fall under next case
         * in order to skip the MixedFraction step */
      default:
        // Start / MixedFraction -> DenominatorOfAnsFraction
        assert(m_currentStep == DivisionCycleStep::Start ||
               m_currentStep == DivisionCycleStep::MixedFraction);
        m_currentStep = DivisionCycleStep::DenominatorOfAnsFraction;
        insertText(Symbol::k_ansAliases.mainAlias());
    }
  } else if (mixedFractionsEnabled) {
    assert(m_divisionCycleWithAns == TrinaryBoolean::False);
    bool editionIn1D = linearMode();
    bool handled = true;
    /* When we are in NOT the "Ans" case, the cycle is the following :
     *   - in 1D: Start -> DenominatorOfEmptyFraction ->
     * NumeratorOfEmptyFraction   -> MixedFraction -> DenominatorOfEmptyFraction
     * -> etc
     *   - in 2D: Start -> NumeratorOfEmptyFraction   ->
     * DenominatorOfEmptyFraction -> MixedFraction -> NumeratorOfEmptyFraction
     * -> etc and we do NOT cycle when the country doesn't enable mixed
     * fractions (because without the mixed fraction step, the cycle would only
     * switch between the numerator and the denominator of an empty fraction,
     * which is not the wanted behavior when pressing the Division key) */
    switch (m_currentStep) {
      case DivisionCycleStep::Start:
        handled = Escher::LayoutField::handleEvent(event);
        /* In 1D we always cycle
         * In 2D we cycle only if the default handleEvent created an empty
         * fraction */
        if (editionIn1D) {
          // 1D: Start -> DenominatorOfEmptyFraction
          m_currentStep = DivisionCycleStep::DenominatorOfEmptyFraction;
        } else if (cursor()->isAtNumeratorOfEmptyFraction()) {
          // 2D: Start -> NumeratorOfEmptyFraction
          m_currentStep = DivisionCycleStep::NumeratorOfEmptyFraction;
        }
        if (!handled) {
          m_divisionCycleWithAns = TrinaryBoolean::Unknown;
        }
        return handled;
      case DivisionCycleStep::NumeratorOfEmptyFraction:
        if (editionIn1D) {
          // 1D: NumeratorOfEmptyFraction -> MixedFraction
          m_currentStep = DivisionCycleStep::MixedFraction;
          handled = Escher::LayoutField::handleEvent(Ion::Events::Space);
          if (!handled) {
            // Not enough space
            return false;
          }
          event = Ion::Events::Left;
        } else {
          // 2D: NumeratorOfEmptyFraction -> DenominatorOfEmptyFraction
          m_currentStep = DivisionCycleStep::DenominatorOfEmptyFraction;
          event = Ion::Events::Down;
        }
        break;
      case DivisionCycleStep::DenominatorOfEmptyFraction:
        if (editionIn1D) {
          // 1D: DenominatorOfEmptyFraction -> NumeratorOfEmptyFraction
          m_currentStep = DivisionCycleStep::NumeratorOfEmptyFraction;
        } else {
          // 2D: DenominatorOfEmptyFraction -> MixedFraction
          m_currentStep = DivisionCycleStep::MixedFraction;
        }
        event = Ion::Events::Left;
        break;
      default:
        assert(m_currentStep == DivisionCycleStep::MixedFraction &&
               mixedFractionsEnabled);
        if (editionIn1D) {
          // 1D: MixedFraction -> DenominatorOfEmptyFraction
          m_currentStep = DivisionCycleStep::DenominatorOfEmptyFraction;
          handled = Escher::LayoutField::handleEvent(Ion::Events::Right);
          assert(handled);
          handled = Escher::LayoutField::handleEvent(Ion::Events::Backspace);
          assert(handled);
        } else {
          // 2D: MixedFraction -> NumeratorOfEmptyFraction
          m_currentStep = DivisionCycleStep::NumeratorOfEmptyFraction;
        }
        event = Ion::Events::Right;
        break;
    }
  }
  return Escher::LayoutField::handleEvent(event);
}

}  // namespace Calculation
