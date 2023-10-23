#include "plot_banner_view.h"

#include <assert.h>

namespace Statistics {

PlotBannerView::PlotBannerView()
    : m_seriesName(k_bannerFieldFormat), m_result(k_bannerFieldFormat) {}

Escher::View* PlotBannerView::subviewAtIndex(int index) {
  assert(0 <= index && index < numberOfSubviews());
  Escher::View* subviews[] = {&m_seriesName, valueView(), &m_result};
  return subviews[index];
}

}  // namespace Statistics
