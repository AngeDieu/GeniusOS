#ifndef ESCHER_ALTERNATE_EMPTY_VIEW_CONTROLLER_H
#define ESCHER_ALTERNATE_EMPTY_VIEW_CONTROLLER_H

#include <escher/alternate_empty_view_delegate.h>
#include <escher/i18n.h>
#include <escher/view_controller.h>
#include <poincare/trinary_boolean.h>

namespace Escher {

class AlternateEmptyViewController : public ViewController {
 public:
  AlternateEmptyViewController(Responder* parentResponder,
                               ViewController* mainViewController,
                               AlternateEmptyViewDelegate* delegate);
  View* view() override { return &m_contentView; }
  const char* title() override;
  ViewController::TitlesDisplay titlesDisplay() override;
  void didBecomeFirstResponder() override;
  void initView() override;
  void viewWillAppear() override;
  void viewDidDisappear() override;

 private:
  class ContentView : public View {
   public:
    ContentView(ViewController* mainViewController,
                AlternateEmptyViewDelegate* delegate);
    ViewController* mainViewController() const { return m_mainViewController; }
    AlternateEmptyViewDelegate* alternateEmptyViewDelegate() const {
      return m_delegate;
    }
    void layoutSubviews(bool force = false) override;
    bool isEmpty();
    void resetIsEmpty() { m_isEmpty = Poincare::TrinaryBoolean::Unknown; }
    View* currentView();

   private:
    int numberOfSubviews() const override { return 1; }
    View* subviewAtIndex(int index) override;
    ViewController* m_mainViewController;
    AlternateEmptyViewDelegate* m_delegate;
    Poincare::TrinaryBoolean m_isEmpty;
  };
  ContentView m_contentView;
};

}  // namespace Escher
#endif
