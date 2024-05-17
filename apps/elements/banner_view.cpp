#include "banner_view.h"

#include <assert.h>
#include <poincare/code_point_layout.h>
#include <poincare/horizontal_layout.h>
#include <poincare/layout_helper.h>

#include "app.h"

using namespace Poincare;

namespace Elements {

// BannerView::DotView

void BannerView::DotView::drawRect(KDContext* ctx, KDRect rect) const {
  ctx->fillAntialiasedCircle(KDPointZero, k_dotDiameter / 2, m_color,
                             k_backgroundColor);
}

void BannerView::DotView::setColor(KDColor color) {
  if (color != m_color) {
    markWholeFrameAsDirty();
    m_color = color;
  }
}

// BannerView

BannerView::BannerView(Escher::Responder* textFieldParent,
                       Escher::TextFieldDelegate* textFieldDelegate)
    : m_textField(textFieldParent, textFieldDelegate),
      m_legendView({.style = {.glyphColor = k_legendColor,
                              .backgroundColor = k_backgroundColor,
                              .font = k_legendSize}}),
      m_button(k_backgroundColor) {
  m_textField.margins()->setLeft(Escher::Metric::CommonSmallMargin);
}

void BannerView::drawRect(KDContext* ctx, KDRect rect) const {
  ctx->fillRect(KDRect(0, 0, bounds().width(), k_borderHeight), k_borderColor);
  ctx->fillRect(KDRect(0, k_borderHeight, bounds().width() - k_buttonWidth,
                       k_bannerHeight),
                k_backgroundColor);
}

void BannerView::reload() {
  if (!(displayTextField() || m_textField.bounds().isEmpty())) {
    /* Text field will disappear, we need to redraw the full background */
    markWholeFrameAsDirty();
  }
  layoutSubviews();
}

Escher::View* BannerView::subviewAtIndex(int index) {
  if (displayTextField()) {
    assert(index == 0);
    return &m_textField;
  }
  switch (index) {
    case 0:
      return &m_dotView;
    case 1:
      return &m_legendView;
    default:
      assert(index == 2);
      return &m_button;
  }
}

void BannerView::layoutSubviews(bool force) {
  if (displayTextField()) {
    setChildFrame(&m_textField,
                  KDRect(0, k_borderHeight, bounds().width(),
                         bounds().height() - k_borderHeight),
                  force);
    return;
  }
  setChildFrame(&m_textField, KDRectZero, force);

  ElementsViewDataSource* dataSource = App::app()->elementsViewDataSource();
  KDCoordinate x = k_dotLeftMargin;

  AtomicNumber z = dataSource->selectedElement();
  KDColor buttonColor;
  if (z != ElementsDataBase::k_noElement) {
    KDRect dotRect = KDRect(x, (bounds().height() - k_dotDiameter) / 2,
                            k_dotDiameter, k_dotDiameter);
    setChildFrame(&m_dotView, dotRect, force);
    m_dotView.setColor(dataSource->field()->getColors(z).fg());
    x += dotRect.width() + k_dotLegendMargin;
    buttonColor = k_backgroundColor;
  } else {
    setChildFrame(&m_dotView, KDRectZero, force);
    buttonColor = k_selectedButtonColor;
  }

  setChildFrame(&m_button,
                KDRect(bounds().width() - k_buttonWidth, k_borderHeight,
                       k_buttonWidth, k_bannerHeight),
                force);
  m_button.setColor(buttonColor);

  I18n::Message legendMessage = dataSource->field()->getMessage(z);
  if (legendMessage != I18n::Message::Default) {
    m_legendView.setLayout(
        LayoutHelper::String(I18n::translate(legendMessage)));
  } else {
    HorizontalLayout h = HorizontalLayout::Builder();
    h.addOrMergeChildAtIndex(LayoutHelper::String(I18n::translate(
                                 dataSource->field()->fieldLegend())),
                             0);
    if (dataSource->field()->hasDouble(z)) {
      h.addOrMergeChildAtIndex(CodePointLayout::Builder(' '),
                               h.numberOfChildren());
      h.addOrMergeChildAtIndex(dataSource->field()->getLayout(z),
                               h.numberOfChildren());
    }
    m_legendView.setLayout(h);
  }
  KDCoordinate bannerBaseline = k_bannerHeight / 2 + 1;
  KDCoordinate legendY = k_borderHeight + bannerBaseline -
                         m_legendView.layout().baseline(k_legendSize);
  setChildFrame(&m_legendView,
                KDRect(x, legendY, bounds().width() - k_buttonWidth - x,
                       m_legendView.layout().layoutSize(k_legendSize).height()),
                force);
}

}  // namespace Elements
