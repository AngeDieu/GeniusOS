#include "right_integral_calculation.h"

#include <assert.h>
#include <poincare/preferences.h>

#include <cmath>

#include "../distribution/distribution.h"

namespace Distributions {

RightIntegralCalculation::RightIntegralCalculation(Distribution* distribution)
    : Calculation(distribution),
      m_lowerBound(distribution->defaultComputedValue()) {
  if (!distribution->allParametersAreInitialized()) {
    m_result = k_medianProbability;
  }
}

I18n::Message RightIntegralCalculation::legendForParameterAtIndex(int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    return I18n::Message::RightIntegralFirstLegend;
  }
  return I18n::Message::IntegralSecondLegend;
}

void RightIntegralCalculation::setParameterAtIndex(double f, int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    m_lowerBound = f;
  }
  if (index == 1) {
    m_result = f;
  }
  if (!m_distribution->allParametersAreInitialized()) {
    computeUnknownDistributionParameter();
    return;
  }
  compute(index);
}

double RightIntegralCalculation::parameterAtIndex(int index) {
  assert(index >= 0 && index < 2);
  if (index == 0) {
    return m_lowerBound;
  }
  return m_result;
}

void RightIntegralCalculation::compute(int indexKnownElement) {
  if (m_distribution == nullptr) {
    return;
  }
  if (indexKnownElement == 0) {
    m_result = m_distribution->rightIntegralFromAbscissa(m_lowerBound);
  } else {
    if (m_distribution->authorizedParameterAtIndex(m_lowerBound, 0)) {
      double currentResult =
          m_distribution->rightIntegralFromAbscissa(m_lowerBound);
      if (std::fabs(currentResult - m_result) <
          std::pow(
              10.0,
              -Poincare::Preferences::VeryLargeNumberOfSignificantDigits)) {
        m_result = currentResult;
        return;
      }
    }
    m_lowerBound = m_distribution->rightIntegralInverseForProbability(m_result);
    m_result = m_distribution->rightIntegralFromAbscissa(m_lowerBound);
    if (std::isnan(m_lowerBound)) {
      m_result = NAN;
    }
  }
}

void RightIntegralCalculation::computeUnknownDistributionParameter() {
  assert(m_distribution->canHaveUninitializedParameter());
  m_distribution->computeUnknownParameterForProbabilityAndBound(
      m_result, m_lowerBound, false);
}

}  // namespace Distributions
