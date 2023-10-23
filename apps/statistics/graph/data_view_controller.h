#ifndef STATISTICS_DATA_VIEW_CONTROLLER_H
#define STATISTICS_DATA_VIEW_CONTROLLER_H

#include <escher/tab_view_controller.h>
#include <escher/view_controller.h>

#include "../store.h"
#include "data_view.h"
#include "graph_button_row_delegate.h"

namespace Statistics {

class DataViewController : public Escher::ViewController,
                           public GraphButtonRowDelegate {
 public:
  DataViewController(Escher::Responder* parentResponder,
                     Escher::TabViewController* tabController,
                     Escher::ButtonRowController* header,
                     Escher::StackViewController* stackViewController,
                     Escher::ViewController* typeViewController, Store* store);
  virtual DataView* dataView() = 0;

  // ViewController
  Escher::View* view() override { return dataView(); }
  void viewWillAppear() override final;

  // Responder
  bool handleEvent(Ion::Events::Event event) override;
  void didEnterResponderChain(
      Escher::Responder* previousFirstResponder) override;
  void willExitResponderChain(Escher::Responder* nextFirstResponder) override;

  int selectedSeries() const;
  void setSelectedSeries(int selectedSeries);
  int selectedIndex() const;
  void setSelectedIndex(int selectedIndex);

 protected:
  /* Some DataViewController implementations may have a stricter condition.
   * To simplify the code, views and ranges of controllers using this default
   * implementation directly use
   * Shared::DoublePairStore::DefaultActiveSeriesTest. They should be updated
   * accordingly if their controllers override this method. */
  virtual Shared::DoublePairStore::ActiveSeriesTest activeSeriesMethod() const {
    return Shared::DoublePairStore::DefaultActiveSeriesTest;
  };
  void sanitizeSeriesIndex();
  virtual void viewWillAppearBeforeReload() {}
  virtual bool reloadBannerView() = 0;
  virtual bool moveSelectionHorizontally(
      OMG::HorizontalDirection direction) = 0;
  virtual bool moveSelectionVertically(OMG::VerticalDirection direction);
  // Overriden with histograms only to highlight and scroll to the selected bar
  virtual void highlightSelection() {}

  virtual int nextSubviewWhenMovingVertically(
      OMG::VerticalDirection direction) const {
    return m_store->activeSeriesIndexFromSeriesIndex(selectedSeries(),
                                                     activeSeriesMethod()) +
           (direction.isUp() ? -1 : 1);
  }
  virtual void updateHorizontalIndexAfterSelectingNewSeries(
      int previousSelectedSeries) = 0;

  Store* m_store;

 private:
  Escher::TabViewController* m_tabController;
};

}  // namespace Statistics

#endif
