#include "additional_results_type.h"

#include <apps/apps_container_helper.h>
#include <poincare/preferences.h>
#include <poincare/trigonometry.h>
#include <poincare/unit.h>

#include "../calculation.h"
#include "scientific_notation_helper.h"
#include "trigonometry_helper.h"
#include "unit_comparison_helper.h"
#include "vector_helper.h"

using namespace Poincare;
using namespace Shared;

namespace Calculation {

AdditionalResultsType AdditionalResultsType::AdditionalResultsForExpressions(
    const Expression input, const Expression exactOutput,
    const Expression approximateOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  if (ForbidAdditionalResults(input, exactOutput, approximateOutput)) {
    return AdditionalResultsType{};
  }
  if (HasComplex(approximateOutput, complexFormat, angleUnit)) {
    return AdditionalResultsType{.complex = true};
  }
  bool inputHasAngleUnit, exactHasAngleUnit, approximateHasAngleUnit;
  bool inputHasUnit = input.hasUnit(true, &inputHasAngleUnit);
  bool exactHasUnit = exactOutput.hasUnit(true, &exactHasAngleUnit);
  bool approximateHasUnit =
      approximateOutput.hasUnit(true, &approximateHasAngleUnit);
  assert(exactHasUnit == approximateHasUnit);
  (void)approximateHasUnit;  // Silence compiler
  if (inputHasUnit || exactHasUnit) {
    /* We display units additional results based on exact output. If input has
     * units but not output (ex: L/(L/3)), we don't display any results. */
    return AdditionalResultsType{
        .unit = exactHasUnit && HasUnit(exactOutput, complexFormat, angleUnit)};
  }
  if (HasDirectTrigo(input, exactOutput, complexFormat, angleUnit)) {
    return AdditionalResultsType{.directTrigonometry = true};
  }
  if (HasInverseTrigo(input, exactOutput, complexFormat, angleUnit)) {
    return AdditionalResultsType{.inverseTrigonometry = true};
  }
  if (HasVector(exactOutput, approximateOutput, complexFormat, angleUnit)) {
    return AdditionalResultsType{.vector = true};
  }
  if (approximateOutput.deepIsMatrix()) {
    return AdditionalResultsType{.matrix = HasMatrix(approximateOutput)};
  }
  AdditionalResultsType type = {};
  if (inputHasAngleUnit || exactHasAngleUnit || approximateHasAngleUnit) {
    // Forbig remaining units (angle units)
    return type;
  }
  if (HasFunction(input, approximateOutput)) {
    type.function = true;
  }
  if (HasScientificNotation(approximateOutput)) {
    type.scientificNotation = true;
  }
  if (HasInteger(exactOutput)) {
    type.integer = true;
  } else if (HasRational(exactOutput)) {
    type.rational = true;
  }
  return type;
}

bool AdditionalResultsType::ForbidAdditionalResults(
    const Expression input, const Expression exactOutput,
    const Expression approximateOutput) {
  /* Special case for Store:
   * Store nodes have to be at the root of the expression, which prevents
   * from creating new expressions with store node as a child. We don't
   * return any additional outputs for them to avoid bothering with special
   * cases. */
  if (Preferences::sharedPreferences->examMode().forbidAdditionalResults() ||
      input.isUninitialized() || exactOutput.isUninitialized() ||
      approximateOutput.isUninitialized() ||
      input.type() == ExpressionNode::Type::Store ||
      exactOutput.type() == ExpressionNode::Type::List ||
      approximateOutput.type() == ExpressionNode::Type::List ||
      approximateOutput.recursivelyMatches([](const Expression e) {
        return e.isUndefined() || e.type() == ExpressionNode::Type::Infinity;
      })) {
    return true;
  }
  assert(!input.isUndefined() && !exactOutput.isUndefined());
  return false;
}

bool AdditionalResultsType::HasComplex(
    const Expression approximateOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  return approximateOutput.isScalarComplex(complexFormat, angleUnit);
}

bool AdditionalResultsType::HasDirectTrigo(
    const Expression input, const Expression exactOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  assert(!exactOutput.hasUnit(true));
  Context *globalContext =
      AppsContainerHelper::sharedAppsContainerGlobalContext();
  Expression exactAngle = TrigonometryHelper::ExtractExactAngleFromDirectTrigo(
      input, exactOutput, globalContext, complexFormat, angleUnit);
  return !exactAngle.isUninitialized();
}

bool AdditionalResultsType::HasInverseTrigo(
    const Expression input, const Expression exactOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  // If the result is complex, it is treated as a complex result instead.
  assert(!exactOutput.isScalarComplex(complexFormat, angleUnit));
  assert(!exactOutput.hasUnit(true));
  return (Trigonometry::IsInverseTrigonometryFunction(input)) ||
         Trigonometry::IsInverseTrigonometryFunction(exactOutput);
}

bool AdditionalResultsType::HasUnit(
    const Expression exactOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  assert(exactOutput.hasUnit(true));
  Context *globalContext =
      AppsContainerHelper::sharedAppsContainerGlobalContext();
  Expression unit;
  Expression clone = exactOutput.clone();
  PoincareHelpers::CloneAndReduceAndRemoveUnit(
      &clone, &unit, globalContext,
      {.complexFormat = complexFormat,
       .angleUnit = angleUnit,
       .symbolicComputation =
           SymbolicComputation::ReplaceAllSymbolsWithDefinitionsOrUndefined,
       .unitConversion = UnitConversion::None});
  double value = PoincareHelpers::ApproximateToScalar<double>(
      clone, globalContext,
      {.complexFormat = complexFormat, .angleUnit = angleUnit});
  if (!unit.isUninitialized() &&
      (Unit::ShouldDisplayAdditionalOutputs(
           value, unit,
           GlobalPreferences::sharedGlobalPreferences->unitFormat()) ||
       UnitComparison::ShouldDisplayUnitComparison(value, unit))) {
    /* Sometimes with angle units, the reduction with UnitConversion::None
     * will be defined but not the reduction with UnitConversion::Default,
     * which will make the unit list controller crash.  */
    unit = Expression();
    clone = exactOutput.clone();
    PoincareHelpers::CloneAndReduceAndRemoveUnit(
        &clone, &unit, globalContext,
        {.complexFormat = complexFormat, .angleUnit = angleUnit});
    return !unit.isUninitialized();
  }
  return false;
}

bool AdditionalResultsType::HasVector(
    const Expression exactOutput, const Expression approximateOutput,
    const Preferences::ComplexFormat complexFormat,
    const Preferences::AngleUnit angleUnit) {
  Context *globalContext =
      AppsContainerHelper::sharedAppsContainerGlobalContext();
  Expression norm = VectorHelper::BuildVectorNorm(
      exactOutput.clone(), globalContext, complexFormat, angleUnit);
  if (norm.isUninitialized()) {
    return false;
  }
  assert(!norm.isUndefined());
  int nChildren = approximateOutput.numberOfChildren();
  for (int i = 0; i < nChildren; ++i) {
    if (approximateOutput.childAtIndex(i).isScalarComplex(complexFormat,
                                                          angleUnit)) {
      return false;
    }
  }
  return true;
}

bool AdditionalResultsType::HasMatrix(const Expression approximateOutput) {
  assert(!approximateOutput.isUninitialized());
  assert(!approximateOutput.hasUnit(true));
  return approximateOutput.type() == ExpressionNode::Type::Matrix &&
         !approximateOutput.recursivelyMatches(Expression::IsUndefined);
}

static bool expressionIsInterestingFunction(const Expression e) {
  assert(!e.isUninitialized());
  if (e.isOfType({ExpressionNode::Type::Opposite,
                  ExpressionNode::Type::Parenthesis})) {
    return expressionIsInterestingFunction(e.childAtIndex(0));
  }
  return !e.isNumber() &&
         !e.isOfType({ExpressionNode::Type::ConstantMaths,
                      ExpressionNode::Type::UnitConvert}) &&
         !e.deepIsOfType({ExpressionNode::Type::Sequence,
                          ExpressionNode::Type::Factor,
                          ExpressionNode::Type::RealPart,
                          ExpressionNode::Type::ImaginaryPart,
                          ExpressionNode::Type::ComplexArgument,
                          ExpressionNode::Type::Conjugate}) &&
         e.numberOfNumericalValues() == 1;
}

bool AdditionalResultsType::HasFunction(const Expression input,
                                        const Expression approximateOutput) {
  // We want a single numerical value and to avoid showing the identity function
  assert(!input.isUninitialized());
  assert(!input.hasUnit());
  assert(!approximateOutput.isUndefined());
  assert(!approximateOutput.hasUnit());
  assert(approximateOutput.type() != ExpressionNode::Type::Matrix);
  return approximateOutput.type() != ExpressionNode::Type::Nonreal &&
         expressionIsInterestingFunction(input);
}

bool AdditionalResultsType::HasScientificNotation(
    const Expression approximateOutput) {
  assert(!approximateOutput.isUninitialized());
  assert(!approximateOutput.hasUnit());
  Context *globalContext =
      AppsContainerHelper::sharedAppsContainerGlobalContext();
  return approximateOutput.type() != ExpressionNode::Type::Nonreal &&
         Preferences::sharedPreferences->displayMode() !=
             Preferences::PrintFloatMode::Scientific &&
         ScientificNotationHelper::HasAdditionalOutputs(approximateOutput,
                                                        globalContext);
}

bool AdditionalResultsType::HasInteger(const Expression exactOutput) {
  assert(!exactOutput.isUninitialized());
  assert(!exactOutput.hasUnit());
  constexpr const char *k_maximalIntegerWithAdditionalResults =
      "10000000000000000";
  return exactOutput.isBasedIntegerCappedBy(
      k_maximalIntegerWithAdditionalResults);
}

bool AdditionalResultsType::HasRational(const Expression exactOutput) {
  // Find forms like [12]/[23] or -[12]/[23]
  assert(!exactOutput.isUninitialized());
  assert(!exactOutput.hasUnit());
  return exactOutput.isDivisionOfIntegers() ||
         (exactOutput.type() == ExpressionNode::Type::Opposite &&
          exactOutput.childAtIndex(0).isDivisionOfIntegers());
}

}  // namespace Calculation
