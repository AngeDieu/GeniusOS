#include <poincare/point_evaluation.h>

#include "poincare/expression.h"
#include "poincare/float.h"
#include "poincare/point.h"

namespace Poincare {

template <typename T>
Expression PointEvaluationNode<T>::complexToExpression(
    Preferences::ComplexFormat complexFormat) const {
  return Point::Builder(Float<T>::Builder(m_x), Float<T>::Builder(m_y));
}

template <typename T>
PointEvaluation<T> PointEvaluation<T>::Builder(T x, T y) {
  void *bufferNode =
      TreePool::sharedPool->alloc(sizeof(PointEvaluationNode<T>));
  PointEvaluationNode<T> *node = new (bufferNode) PointEvaluationNode<T>(x, y);
  TreeHandle h = TreeHandle::BuildWithGhostChildren(node);
  return static_cast<PointEvaluation<T> &>(h);
}

template PointEvaluation<float> PointEvaluation<float>::Builder(float x,
                                                                float y);
template PointEvaluation<double> PointEvaluation<double>::Builder(double x,
                                                                  double y);

}  // namespace Poincare
