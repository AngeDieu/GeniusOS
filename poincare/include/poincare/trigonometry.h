#ifndef POINCARE_TRIGONOMETRY_H
#define POINCARE_TRIGONOMETRY_H

#include <poincare/evaluation.h>
#include <poincare/expression.h>

namespace Poincare {

class Trigonometry final {
 public:
  enum class Function {
    Cosine = 0,
    Sine = 1,
  };
  static Expression AnglePeriodInAngleUnit(Preferences::AngleUnit angleUnit);
  static Expression PiExpressionInAngleUnit(Preferences::AngleUnit angleUnit);
  static double PiInAngleUnit(Preferences::AngleUnit angleUnit);
  static bool isDirectTrigonometryFunction(const Expression& e);
  static bool isInverseTrigonometryFunction(const Expression& e);
  static bool isAdvancedTrigonometryFunction(const Expression& e);
  static bool isInverseAdvancedTrigonometryFunction(const Expression& e);
  static bool AreInverseFunctions(const Expression& directFunction,
                                  const Expression& inverseFunction);
  /* Returns a (unreduced) division between pi in each unit, or 1 if the units
   * are the same. */
  static Expression UnitConversionFactor(Preferences::AngleUnit fromUnit,
                                         Preferences::AngleUnit toUnit);
  static bool ExpressionIsEquivalentToTangent(const Expression& e);
  static bool ExpressionIsEquivalentToInverseOfTangent(const Expression& e);
  static Expression shallowReduceDirectFunction(
      Expression& e, ReductionContext reductionContext);
  static Expression shallowReduceInverseFunction(
      Expression& e, ReductionContext reductionContext);
  static Expression shallowReduceAdvancedFunction(
      Expression& e, ReductionContext reductionContext);
  static Expression shallowReduceInverseAdvancedFunction(
      Expression& e, ReductionContext reductionContext);
  static Expression replaceWithAdvancedFunction(Expression& e,
                                                Expression& denominator);
  // , Function f, bool inverse
  static Expression table(const Expression e, ExpressionNode::Type type,
                          const ReductionContext& reductionContext);
  template <typename T>
  static std::complex<T> ConvertToRadian(const std::complex<T> c,
                                         Preferences::AngleUnit angleUnit);
  template <typename T>
  static std::complex<T> ConvertRadianToAngleUnit(
      const std::complex<T> c, Preferences::AngleUnit angleUnit);

  /* Detect if expression is of the form A*cosOrSin(Bx+C) + K
   * The return coefficient is A.
   * The returned angle is the value between 0 and 2pi such as the expression
   * is of the form A*cos(Bx+angle) + K
   * K can be non-null only if acceptAddition = true
   *
   * TODO: This function is very specific and used only in poincare/conic.cpp
   * and shared/continuous_function_properties.cpp to detect some precise
   * function patterns. It needs a refactor and could maybe be factorized with
   * Expression::isLinearCombinationOfFunction */
  static bool DetectLinearPatternOfCosOrSin(
      const Expression& e, ReductionContext reductionContext,
      const char* symbol, bool acceptAddition,
      double* coefficientBeforeCos = nullptr,
      double* coefficientBeforeSymbol = nullptr, double* angle = nullptr);

  /* Turn cos(4) into cos(4rad) if the angle unit is rad and cos(π) into
   * cos(π°) if the angle unit is deg, to notify the user of the current
   * angle unit she is using if she's forgetting to switch the angle unit */
  static Expression DeepAddAngleUnitToAmbiguousDirectFunctions(
      Expression& e, Preferences::AngleUnit angleUnit);

 private:
  static bool ExpressionIsTangentOrInverseOfTangent(const Expression& e,
                                                    bool inverse);
};

}  // namespace Poincare

#endif
