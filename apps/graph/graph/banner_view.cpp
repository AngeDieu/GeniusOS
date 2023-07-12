#include "banner_view.h"
#include <assert.h>
#include <apps/i18n.h>

using namespace Escher;

namespace Graph {

BannerView::BannerView(
  Responder * parentResponder,
  InputEventHandlerDelegate * inputEventHandlerDelegate,
  TextFieldDelegate * textFieldDelegate
) :
  Shared::XYBannerView(parentResponder, inputEventHandlerDelegate, textFieldDelegate),
  m_derivativeView(k_font, KDContext::k_alignCenter, KDContext::k_alignCenter, TextColor(), BackgroundColor()),
  m_tangentEquationView(k_font, I18n::Message::LinearRegressionFormula, KDContext::k_alignCenter, KDContext::k_alignCenter, TextColor(), BackgroundColor()),
  m_aView(k_font, KDContext::k_alignCenter, KDContext::k_alignCenter, TextColor(), BackgroundColor()),
  m_bView(k_font, KDContext::k_alignCenter, KDContext::k_alignCenter, TextColor(), BackgroundColor())
{
  for (int i = 0; i < k_maxNumberOfInterests; i++) {
    m_interestMessageView[i] = MessageTextView(k_font, I18n::Message::Default, KDContext::k_alignCenter, KDContext::k_alignCenter, TextColor(), BackgroundColor());
  }
}

void BannerView::addInterestMessage(I18n::Message message, Shared::CursorView * cursor) {
  int n = numberOfInterestMessages();
  if (n == k_maxNumberOfInterests) {
    return;
  }
  m_interestMessageView[n].setMessage(message);
  cursor->setHighlighted(true);
}

void BannerView::emptyInterestMessages(Shared::CursorView * cursor) {
  for (int i = 0; i < k_maxNumberOfInterests; i++) {
    m_interestMessageView[i].setMessage(I18n::Message::Default);
  }
  cursor->setHighlighted(false);
}


void BannerView::setDisplayParameters(bool showInterest, bool showDerivative, bool showTangent) {
  m_showInterest = showInterest;
  m_showDerivative = showDerivative;
  m_showTangent = showTangent;
}

View * BannerView::subviewAtIndex(int index) {
  assert(0 <= index && index < numberOfSubviews());
  int n = numberOfInterestMessages();
  if (index < n) {
    return m_interestMessageView + index;
  }
  index -= n;

  if (index < Shared::XYBannerView::k_numberOfSubviews) {
    return Shared::XYBannerView::subviewAtIndex(index);
  }
  View * subviews[] = {&m_derivativeView, &m_tangentEquationView, &m_aView, &m_bView};
  return subviews[index - Shared::XYBannerView::k_numberOfSubviews];
}

bool BannerView::lineBreakBeforeSubview(View * subview) const {
  return subview == &m_tangentEquationView
      || Shared::XYBannerView::lineBreakBeforeSubview(subview);
}

int BannerView::numberOfInterestMessages() const {
  int result = 0;
  for (int i = 0; i < k_maxNumberOfInterests; i++) {
    if (hasInterestMessage(i)) {
      assert(result == i); // Interests messages should be filled in order
      result++;
    }
  }
  return result;
}

}
