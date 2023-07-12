#ifndef CALCULATION_CALCULATION_H
#define CALCULATION_CALCULATION_H

#include <apps/constant.h>
#include <poincare/context.h>
#include <poincare/expression.h>
#include "../shared/poincare_helpers.h"

namespace Calculation {

class CalculationStore;


/* A calculation is:
 *  |     uint8_t   |KDCoordinate|  KDCoordinate  |  uint8_t  |   ...     |      ...       |         ...           |          ...          |
 *  |m_displayOutput|  m_height  |m_expandedHeight|m_equalSign|m_inputText|m_exactOuputText|m_approximateOuputText1|m_approximateOuputText2|
 *                                                                                               with maximal           with displayed
 *                                                                                            significant digits      significant digits
 *
 * */

class Calculation {
friend CalculationStore;
public:
  constexpr static int k_numberOfExpressions = 4;
  enum class EqualSign : uint8_t {
    Unknown,
    Approximation,
    Equal
  };

  enum class DisplayOutput : uint8_t {
    Unknown,
    ExactOnly,
    ApproximateOnly,
    ExactAndApproximate,
    ExactAndApproximateToggle
  };

  struct AdditionalInformations {
    bool integer : 1;
    bool rational : 1;
    bool directTrigonometry : 1;
    bool inverseTrigonometry : 1;
    bool unit : 1;
    bool matrix : 1;
    bool vector : 1;
    bool complex : 1;
    bool function : 1;
    bool scientificNotation : 1;
    bool isEmpty() const { return !(integer || rational || directTrigonometry || inverseTrigonometry || unit || matrix || vector || complex || function || scientificNotation); }
  };

  static bool DisplaysExact(DisplayOutput d) { return d != DisplayOutput::ApproximateOnly; }

  /* It is not really the minimal size, but it clears enough space for most
   * calculations instead of clearing less space, then fail to serialize, clear
   * more space, fail to serialize, clear more space, etc., until reaching
   * sufficient free space. */
  constexpr static int k_minimalSize = sizeof(uint8_t) + 2*sizeof(KDCoordinate) + sizeof(uint8_t) + k_numberOfExpressions*Constant::MaxSerializedExpressionSize;

  Calculation() :
    m_displayOutput(DisplayOutput::Unknown),
    m_height(-1),
    m_expandedHeight(-1),
    m_equalSign(EqualSign::Unknown)
  {
    assert(sizeof(m_inputText) == 0);
  }
  bool operator==(const Calculation& c);
  Calculation * next() const;

  // Texts
  enum class NumberOfSignificantDigits {
    Maximal,
    UserDefined
  };
  const char * inputText() const { return m_inputText; }
  const char * exactOutputText() const { return m_inputText + strlen(m_inputText) + 1; }
  // See comment in approximateOutput implementation explaining the need of two approximateOutputTexts
  const char * approximateOutputText(NumberOfSignificantDigits numberOfSignificantDigits) const;

  // Expressions
  Poincare::Expression input();
  Poincare::Expression exactOutput();
  Poincare::Expression approximateOutput(NumberOfSignificantDigits numberOfSignificantDigits);

  // Layouts
  Poincare::Layout createInputLayout();
  Poincare::Layout createExactOutputLayout(bool * couldNotCreateExactLayout);
  Poincare::Layout createApproximateOutputLayout(bool * couldNotCreateApproximateLayout);

  // Heights
  KDCoordinate height(bool expanded);

  // Displayed output
  DisplayOutput displayOutput(Poincare::Context * context);
  void forceDisplayOutput(DisplayOutput d);
  EqualSign exactAndApproximateDisplayedOutputsAreEqual(Poincare::Context * context);

  // Additional Informations
  AdditionalInformations additionalInformations();
private:
  constexpr static KDCoordinate k_heightComputationFailureHeight = 50;
  constexpr static const char * k_maximalIntegerWithAdditionalInformation = "10000000000000000";

  void setHeights(KDCoordinate height, KDCoordinate expandedHeight);

  /* Buffers holding text expressions have to be longer than the text written
   * by user (of maximum length TextField::MaxBufferSize()) because when we
   * print an expression we add omitted signs (multiplications, parenthesis...) */
  DisplayOutput m_displayOutput;
  KDCoordinate m_height __attribute__((packed));
  KDCoordinate m_expandedHeight __attribute__((packed));
  EqualSign m_equalSign;
  char m_inputText[0]; // MUST be the last member variable
};

}

#endif
