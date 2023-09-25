#ifndef SHARED_POINCARE_HELPERS_H
#define SHARED_POINCARE_HELPERS_H

#include <apps/global_preferences.h>
#include <poincare/expression.h>
#include <poincare/preferences.h>
#include <poincare/print_float.h>
#include <poincare/solver.h>

namespace Shared {

namespace PoincareHelpers {

constexpr static Poincare::SymbolicComputation
    k_replaceWithDefinitionOrUndefined = Poincare::SymbolicComputation::
        ReplaceAllSymbolsWithDefinitionsOrUndefined;
constexpr static Poincare::SymbolicComputation k_replaceWithDefinition =
    Poincare::SymbolicComputation::ReplaceAllDefinedSymbolsWithDefinition;
constexpr static Poincare::UnitConversion k_defaultUnitConversion =
    Poincare::UnitConversion::Default;

inline Poincare::Layout CreateLayout(
    const Poincare::Expression e, Poincare::Context* context,
    Poincare::Preferences* preferences =
        Poincare::Preferences::sharedPreferences) {
  return e.createLayout(preferences->displayMode(),
                        preferences->numberOfSignificantDigits(), context);
}

template <class T>
inline int ConvertFloatToText(T d, char* buffer, int bufferSize,
                              int numberOfSignificantDigits) {
  return Poincare::PrintFloat::ConvertFloatToText(
             d, buffer, bufferSize,
             Poincare::PrintFloat::glyphLengthForFloatWithPrecision(
                 numberOfSignificantDigits),
             numberOfSignificantDigits,
             Poincare::Preferences::sharedPreferences->displayMode())
      .CharLength;
}

template <class T>
inline int ConvertFloatToTextWithDisplayMode(
    T d, char* buffer, int bufferSize, int numberOfSignificantDigits,
    Poincare::Preferences::PrintFloatMode displayMode) {
  return Poincare::PrintFloat::ConvertFloatToText(
             d, buffer, bufferSize,
             Poincare::PrintFloat::glyphLengthForFloatWithPrecision(
                 numberOfSignificantDigits),
             numberOfSignificantDigits, displayMode)
      .CharLength;
}

inline int Serialize(
    const Poincare::Expression e, char* buffer, int bufferSize,
    int numberOfSignificantDigits =
        Poincare::PrintFloat::k_numberOfStoredSignificantDigits) {
  return e.serialize(buffer, bufferSize,
                     Poincare::Preferences::sharedPreferences->displayMode(),
                     numberOfSignificantDigits);
}

inline Poincare::ReductionContext ReductionContextForParameters(
    const Poincare::Expression e, Poincare::Context* context,
    Poincare::Preferences::ComplexFormat complexFormat,
    Poincare::ReductionTarget target,
    Poincare::SymbolicComputation symbolicComputation,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion,
    bool updateComplexFormat) {
  Poincare::ReductionContext reductionContext(
      context, complexFormat,
      Poincare::Preferences::sharedPreferences->angleUnit(),
      GlobalPreferences::sharedGlobalPreferences->unitFormat(), target,
      symbolicComputation, unitConversion);
  reductionContext.updateComplexFormat(updateComplexFormat, e);
  return reductionContext;
}

inline Poincare::ApproximationContext ApproximationContextForParameters(
    const Poincare::Expression e, Poincare::Context* context) {
  Poincare::ApproximationContext approximationContext(context);
  approximationContext.updateComplexFormat(true, e);
  return approximationContext;
}

template <class T>
inline Poincare::Expression Approximate(const Poincare::Expression e,
                                        Poincare::Context* context) {
  return e.approximate<T>(ApproximationContextForParameters(e, context));
}

template <class T>
inline T ApproximateToScalar(const Poincare::Expression e,
                             Poincare::Context* context) {
  return e.approximateToScalar<T>(
      ApproximationContextForParameters(e, context));
}

template <class T>
inline T ApproximateWithValueForSymbol(const Poincare::Expression e,
                                       const char* symbol, T x,
                                       Poincare::Context* context) {
  return e.approximateWithValueForSymbol<T>(
      symbol, x, ApproximationContextForParameters(e, context));
}

template <class T>
inline Poincare::Expression ApproximateKeepingUnits(
    const Poincare::Expression e, Poincare::Context* context,
    Poincare::ReductionTarget target = Poincare::ReductionTarget::User,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion) {
  return e.approximateKeepingUnits<T>(ReductionContextForParameters(
      e, context, Poincare::Preferences::sharedPreferences->complexFormat(),
      target, symbolicComputation, unitConversion, true));
}

inline void CloneAndSimplify(
    Poincare::Expression* e, Poincare::Context* context,
    Poincare::ReductionTarget target,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion,
    Poincare::Preferences* preferences =
        Poincare::Preferences::sharedPreferences,
    bool updateComplexFormat = true, bool* reductionFailure = nullptr) {
  *e = e->cloneAndSimplify(
      ReductionContextForParameters(
          *e, context,
          Poincare::Preferences::sharedPreferences->complexFormat(), target,
          symbolicComputation, unitConversion, updateComplexFormat),
      reductionFailure);
}

inline void CloneAndSimplifyAndApproximate(
    Poincare::Expression e, Poincare::Expression* simplifiedExpression,
    Poincare::Expression* approximatedExpression, Poincare::Context* context,
    Poincare::Preferences::ComplexFormat complexFormat,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion) {
  Poincare::ReductionContext reductionContext(
      context, complexFormat,
      Poincare::Preferences::sharedPreferences->angleUnit(),
      GlobalPreferences::sharedGlobalPreferences->unitFormat(),
      Poincare::ReductionTarget::User, symbolicComputation, unitConversion);
  reductionContext.updateComplexFormat(true, e);
  e.cloneAndSimplifyAndApproximate(simplifiedExpression, approximatedExpression,
                                   reductionContext);
}

inline void CloneAndReduce(
    Poincare::Expression* e, Poincare::Context* context,
    Poincare::Preferences::ComplexFormat complexFormat,
    Poincare::ReductionTarget target,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion) {
  *e = e->cloneAndReduce(ReductionContextForParameters(
      *e, context, complexFormat, target, symbolicComputation, unitConversion,
      false));
}

inline void CloneAndApproximateKeepingSymbols(
    Poincare::Expression* e, Poincare::Context* context,
    Poincare::Preferences::ComplexFormat complexFormat,
    Poincare::ReductionTarget target,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion) {
  *e = e->cloneAndApproximateKeepingSymbols(ReductionContextForParameters(
      *e, context, complexFormat, target, symbolicComputation, unitConversion,
      false));
}

inline void CloneAndReduceAndRemoveUnit(
    Poincare::Expression* e, Poincare::Expression* unit,
    Poincare::Context* context,
    Poincare::ReductionTarget target = Poincare::ReductionTarget::User,
    Poincare::SymbolicComputation symbolicComputation = k_replaceWithDefinition,
    Poincare::UnitConversion unitConversion = k_defaultUnitConversion) {
  *e = e->cloneAndReduceAndRemoveUnit(
      ReductionContextForParameters(
          *e, context,
          Poincare::Preferences::sharedPreferences->complexFormat(), target,
          symbolicComputation, unitConversion, true),
      unit);
}

// This method automatically updates complex format and angle unit
inline void ParseAndSimplifyAndApproximate(
    const char* text, Poincare::Expression* parsedExpression,
    Poincare::Expression* simplifiedExpression,
    Poincare::Expression* approximateExpression, Poincare::Context* context,
    Poincare::SymbolicComputation symbolicComputation =
        k_replaceWithDefinitionOrUndefined,
    Poincare::Preferences* preferences =
        Poincare::Preferences::sharedPreferences) {
  Poincare::Expression::ParseAndSimplifyAndApproximate(
      text, parsedExpression, simplifiedExpression, approximateExpression,
      context, preferences->complexFormat(), preferences->angleUnit(),
      GlobalPreferences::sharedGlobalPreferences->unitFormat(),
      symbolicComputation);
}

// This method automatically updates complex format and angle unit
template <class T>
inline T ParseAndSimplifyAndApproximateToScalar(
    const char* text, Poincare::Context* context,
    Poincare::SymbolicComputation symbolicComputation =
        k_replaceWithDefinitionOrUndefined,
    Poincare::Preferences* preferences =
        Poincare::Preferences::sharedPreferences) {
  return Poincare::Expression::ParseAndSimplifyAndApproximateToScalar<T>(
      text, context, preferences->complexFormat(), preferences->angleUnit(),
      GlobalPreferences::sharedGlobalPreferences->unitFormat(),
      symbolicComputation);
}

template <typename T>
inline Poincare::Solver<T> Solver(T xMin, T xMax, const char* unknown = nullptr,
                                  Poincare::Context* context = nullptr) {
  Poincare::Preferences* preferences = Poincare::Preferences::sharedPreferences;
  return Poincare::Solver<T>(xMin, xMax, unknown, context,
                             preferences->complexFormat(),
                             preferences->angleUnit());
}

// Return the nearest number from t's representation with given precision.
template <class T>
inline T ValueOfFloatAsDisplayed(T t, int precision,
                                 Poincare::Context* context) {
  assert(precision <= Poincare::PrintFloat::k_numberOfStoredSignificantDigits);
  constexpr static size_t bufferSize =
      Poincare::PrintFloat::charSizeForFloatsWithPrecision(
          Poincare::PrintFloat::k_numberOfStoredSignificantDigits);
  char buffer[bufferSize];
  // Get displayed value
  size_t numberOfChar = ConvertFloatToText<T>(t, buffer, bufferSize, precision);
  assert(numberOfChar <= bufferSize);
  // Silence compiler warnings for assert
  (void)numberOfChar;
  // Extract displayed value
  return ParseAndSimplifyAndApproximateToScalar<T>(buffer, context);
}

}  // namespace PoincareHelpers

}  // namespace Shared

#endif
