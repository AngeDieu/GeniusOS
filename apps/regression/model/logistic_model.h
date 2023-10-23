#ifndef REGRESSION_LOGISTIC_MODEL_H
#define REGRESSION_LOGISTIC_MODEL_H

#include "model.h"

namespace Regression {

class LogisticModel : public Model {
 public:
  using Model::Model;

  I18n::Message formulaMessage() const override {
    return I18n::Message::LogisticRegressionFormula;
  }
  I18n::Message name() const override { return I18n::Message::Logistic; }
  int numberOfCoefficients() const override { return 3; }

  Poincare::Layout templateLayout() const override;

  double evaluate(double* modelCoefficients, double x) const override;
  double levelSet(double* modelCoefficients, double xMin, double xMax, double y,
                  Poincare::Context* context) override;

 private:
  Poincare::Expression privateExpression(
      double* modelCoefficients) const override;
  double partialDerivate(double* modelCoefficients,
                         int derivateCoefficientIndex, double x) const override;
  void specializedInitCoefficientsForFit(double* modelCoefficients,
                                         double defaultValue, Store* store,
                                         int series) const override;
};

}  // namespace Regression

#endif
