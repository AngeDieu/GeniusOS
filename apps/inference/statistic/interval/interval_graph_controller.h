#ifndef INFERENCE_STATISTIC_INTERVAL_INTERVAL_GRAPH_CONTROLLER_H
#define INFERENCE_STATISTIC_INTERVAL_INTERVAL_GRAPH_CONTROLLER_H

#include <escher/stack_view_controller.h>
#include <escher/view_controller.h>

#include "inference/constants.h"
#include "inference/models/statistic_buffer.h"
#include "inference/statistic/interval/interval_graph_view.h"

namespace Inference {

class IntervalGraphController : public Escher::ViewController {
 public:
  IntervalGraphController(Escher::StackViewController* stack,
                          Interval* interval);
  ViewController::TitlesDisplay titlesDisplay() override;
  const char* title() override;
  Escher::View* view() override { return &m_graphView; }
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;
  void setResultTitleForCurrentValues(char* buffer, size_t bufferSize,
                                      bool resultIsTopPage) const;

 private:
  constexpr static int k_titleBufferSize =
      sizeof("ME=") + Constants::k_shortFloatNumberOfChars;
  void resetSelectedInterval();
  void selectAdjacentInterval(bool goUp);
  void saveIntervalValues();
  void intervalDidChange();

  char m_titleBuffer[k_titleBufferSize];
  IntervalGraphView m_graphView;
  Interval* m_interval;
  double m_currentEstimate;
  double m_currentMarginOfError;
  double m_currentThreshold;
  int m_selectedIntervalIndex;
};

}  // namespace Inference
#endif
