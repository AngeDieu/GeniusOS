#include "distribution_cell.h"

using namespace Escher;

namespace Distributions {

DistributionCell::DistributionCell() :
    TableCell(),
    m_messageView(KDFont::Size::Large, (I18n::Message)0, KDContext::k_alignLeft, KDContext::k_alignCenter, KDColorBlack, KDColorWhite)
{
  m_iconView.setBackgroundColor(KDColorWhite);
}

void DistributionCell::reloadCell() {
  HighlightCell::reloadCell();
  KDColor backgroundColor = defaultBackgroundColor();
  m_messageView.setBackgroundColor(backgroundColor);
  m_iconView.setBackgroundColor(backgroundColor);
}

void DistributionCell::setLabel(I18n::Message message) {
  m_messageView.setMessage(message);
}

void DistributionCell::setImage(const Image * image) {
  m_iconView.setImage(image);
}

}  // namespace Distributions
