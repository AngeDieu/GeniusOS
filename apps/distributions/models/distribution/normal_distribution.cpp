#include "normal_distribution.h"

#include <float.h>
#include <poincare/layout_helper.h>

#include <cmath>

using namespace Shared;

namespace Distributions {

bool NormalDistribution::authorizedParameterAtIndex(double x, int index) const {
  if (!TwoParametersDistribution::authorizedParameterAtIndex(x, index)) {
    return false;
  }
  if (index == 0) {
    return true;
  }
  if (x <= DBL_MIN || std::fabs(m_parameters[0] / x) > k_maxRatioMuSigma) {
    return false;
  }
  return true;
}

void NormalDistribution::setParameterAtIndex(double f, int index) {
  setParameterAtIndexWithoutComputingCurveViewRange(f, index);
  if (index == 0 &&
      std::fabs(m_parameters[0] / m_parameters[1]) > k_maxRatioMuSigma) {
    m_parameters[1] = m_parameters[0] / k_maxRatioMuSigma;
  }
  computeCurveViewRange();
}

float NormalDistribution::privateXExtremum(bool min) const {
  assert(!std::isnan(m_parameters[0]) && !std::isnan(m_parameters[1]));
  int coefficient = (min ? -1 : 1);
  if (m_parameters[1] == 0.0f) {
    return m_parameters[0] + coefficient * 1.0f;
  }

  return m_parameters[0] + coefficient * 5.0f * std::fabs(m_parameters[1]);
}

ParameterRepresentation NormalDistribution::paramRepresentationAtIndex(
    int i) const {
  switch (i) {
    case ParamsOrder::Mu: {
      Poincare::Layout u =
          Poincare::LayoutHelper::String(parameterNameAtIndex(ParamsOrder::Mu));
      return ParameterRepresentation{u, I18n::Message::MeanDefinition};
    }
    case ParamsOrder::Sigma: {
      Poincare::Layout s = Poincare::LayoutHelper::String(
          parameterNameAtIndex(ParamsOrder::Sigma));
      return ParameterRepresentation{s, I18n::Message::StandardDeviation};
    }
    default:
      assert(false);
      return ParameterRepresentation{};
  }
}

float NormalDistribution::computeYMax() const {
  float maxAbscissa = m_parameters[0];
  float result = evaluateAtAbscissa(maxAbscissa);
  if (std::isnan(result) || result <= 0.0f) {
    result = 1.0f;
  }
  return result * (1.0f + k_displayTopMarginRatio);
}

}  // namespace Distributions
