#ifndef INFERENCE_MODELS_STATISTIC_ONE_MEAN_T_TEST_H
#define INFERENCE_MODELS_STATISTIC_ONE_MEAN_T_TEST_H

#include "interfaces/distributions.h"
#include "interfaces/significance_tests.h"
#include "test.h"

namespace Inference {

class OneMeanTTest : public Test {
 public:
  SignificanceTestType significanceTestType() const override {
    return SignificanceTestType::OneMean;
  }
  DistributionType distributionType() const override {
    return DistributionType::T;
  }
  I18n::Message title() const override { return OneMean::TTitle(); }

  // Significance Test: OneMean
  bool initializeDistribution(DistributionType distributionType) override {
    return OneMean::TestInitializeDistribution(this, distributionType);
  }
  int numberOfAvailableDistributions() const override {
    return OneMean::NumberOfAvailableDistributions();
  }
  I18n::Message distributionTitle() const override {
    return OneMean::DistributionTitle();
  }
  const char* hypothesisSymbol() override {
    return OneMean::HypothesisSymbol();
  }
  void initParameters() override { OneMean::InitTestParameters(this); }
  bool authorizedParameterAtIndex(double p, int i) const override {
    return Inference::authorizedParameterAtIndex(p, i) &&
           OneMean::TAuthorizedParameterAtIndex(i, p);
  }
  void setParameterAtIndex(double p, int index) override {
    p = OneMean::ProcessParamaterForIndex(p, index);
    Test::setParameterAtIndex(p, index);
  }

  void compute() override { OneMean::ComputeTTest(this); }

 private:
  // Significance Test: OneMean
  int numberOfStatisticParameters() const override {
    return OneMean::NumberOfParameters();
  }
  Shared::ParameterRepresentation paramRepresentationAtIndex(
      int i) const override {
    return OneMean::TParameterRepresentationAtIndex(i);
  }
  double* parametersArray() override { return m_params; }

  double m_params[OneMean::k_numberOfParams];
};

}  // namespace Inference

#endif
