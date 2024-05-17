#ifndef REGRESSION_BANNER_VIEW_H
#define REGRESSION_BANNER_VIEW_H

#include <apps/shared/xy_banner_view.h>
#include <escher/buffer_text_view.h>
#include <escher/message_text_view.h>

namespace Regression {

/* This banner view displays cursor's x and y position as well as (optionally)
 * two other views :
 *  - m_otherView : A buffer text either postionned first or after ordinateView
 *  - m_dataNotSuitableView : A message text displayed last */

class BannerView : public Shared::XYBannerView {
 public:
  BannerView(Escher::Responder* parentResponder,
             Escher::TextFieldDelegate* textFieldDelegate);
  BannerBufferTextView* otherView() { return &m_otherView; }
  void setDisplayParameters(bool displayOtherView, bool otherViewIsFirst,
                            bool displayDataNotSuitable);

 private:
  int numberOfSubviews() const override {
    return XYBannerView::k_numberOfSubviews + m_displayOtherView +
           m_displayDataNotSuitable;
  }
  Escher::View* subviewAtIndex(int index) override;

  BannerBufferTextView m_otherView;
  Escher::MessageTextView m_dataNotSuitableView;
  bool m_displayOtherView;
  bool m_otherViewIsFirst;
  bool m_displayDataNotSuitable;
};

}  // namespace Regression

#endif
