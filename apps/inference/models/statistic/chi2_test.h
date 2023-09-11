#ifndef INFERENCE_MODELS_STATISTIC_CHI2_TEST_H
#define INFERENCE_MODELS_STATISTIC_CHI2_TEST_H

#include <poincare/horizontal_layout.h>

#include <cmath>

#include "table.h"
#include "test.h"

namespace Inference {

class Chi2Test : public Test, public Table {
 public:
  Chi2Test();

  SignificanceTestType significanceTestType() const override {
    return SignificanceTestType::Categorical;
  }
  DistributionType distributionType() const override {
    return DistributionType::Chi2;
  }
  bool initializeCategoricalType(CategoricalType type);

  // Table
  void setParameterAtPosition(double value, int row, int column) override {
    assert(index2DToIndex(row, column) < numberOfStatisticParameters());
    setParameterAtIndex(value, index2DToIndex(row, column));
  }
  double parameterAtPosition(int row, int column) const override {
    return parameterAtIndex(index2DToIndex(row, column));
  }
  bool authorizedParameterAtPosition(double p, int row,
                                     int column) const override {
    return authorizedParameterAtIndex(p, index2DToIndex(row, column));
  }

  bool authorizedParameterAtIndex(double p, int i) const override;

 protected:
  using Test::parameterAtIndex;  // Hidden

  // Instanciate unused abstract classs
  Shared::ParameterRepresentation paramRepresentationAtIndex(
      int i) const override {
    return Shared::ParameterRepresentation{
        Poincare::HorizontalLayout::Builder(), I18n::Message::Default};
  }

  // Chi2 specific
  virtual double expectedValue(int index) const = 0;
  virtual double observedValue(int index) const = 0;
  virtual int numberOfValuePairs() const = 0;
  double computeChi2();

 private:
  // Inference
  float computeXMin() const override {
    return DistributionChi2::XMin(m_degreesOfFreedom);
  }
  float computeXMax() const override {
    return DistributionChi2::XMax(m_degreesOfFreedom);
  }
};

}  // namespace Inference

#endif
