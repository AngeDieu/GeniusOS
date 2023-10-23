#ifndef INFERENCE_STATISTIC_INTERVAL_INTERVAL_CONCLUSION_VIEW_H
#define INFERENCE_STATISTIC_INTERVAL_INTERVAL_CONCLUSION_VIEW_H

#include <escher/buffer_text_view.h>
#include <escher/horizontal_or_vertical_layout.h>
#include <escher/message_text_view.h>

#include "inference/constants.h"

namespace Inference {

class IntervalConclusionView
    : public Escher::MultipleLinesBufferTextView<KDFont::Size::Large, 2> {
 public:
  IntervalConclusionView();
  void setInterval(double center, double marginOfError);
  void reload() { markWholeFrameAsDirty(); }
};

}  // namespace Inference

#endif
