#ifndef ESCHER_DROPDOWN_VIEW_H
#define ESCHER_DROPDOWN_VIEW_H

#include <escher/bordered.h>
#include <escher/bordering_view.h>
#include <escher/image_view.h>
#include <escher/list_view_data_source.h>
#include <escher/responder.h>
#include <escher/selectable_list_view_controller.h>
#include <escher/selectable_table_view.h>
#include <escher/transparent_image_view.h>
#include <escher/view.h>
#include <ion/events.h>

namespace Escher {

// TODO: refactor

/* Wraps a HighlightCell to add margins and an optional caret. */
class PopupItemView : public HighlightCell, public Bordered {
 public:
  PopupItemView(HighlightCell* cell = nullptr);
  void setHighlighted(bool highlighted) override;
  KDSize minimalSizeForOptimalDisplay() const override;
  void layoutSubviews(bool force) override;
  int numberOfSubviews() const override;
  View* subviewAtIndex(int i) override;
  HighlightCell* innerCell() { return m_cell; }
  void setInnerCell(HighlightCell* cell) { m_cell = cell; }
  void drawRect(KDContext* ctx, KDRect rect) const override;
  void setPopping(bool popping) { m_isPoppingUp = popping; }

 protected:
  bool m_isPoppingUp;

 private:
  constexpr static int k_marginCaretRight = 2;
  constexpr static int k_marginImageHorizontal = 3;
  constexpr static int k_marginImageVertical = 2;
  HighlightCell* m_cell;
  TransparentImageView m_caret;
};

class DropdownCallback {
 public:
  virtual void onDropdownSelected(int selectedRow) = 0;
  virtual bool popupDidReceiveEvent(Ion::Events::Event event,
                                    Responder* responder) {
    return false;
  }
};

/* A Dropdown is a view that, when clicked on, displays a list of views to
 * choose from It requires a DropdownDataSource to provide a list of views */
class Dropdown : public PopupItemView, public Responder {
 public:
  Dropdown(Responder* parentResponder,
           ExplicitListViewDataSource* listDataSource,
           DropdownCallback* callback = nullptr);
  Responder* responder() override { return this; }
  bool handleEvent(Ion::Events::Event e) override;
  void reloadAllCells();
  void init();
  void selectRow(int row);

  void open();
  void close();

 private:
  /* List of PopupViews shown in a modal view + Wraps a ListViewDataSource to
   * return PopupViews. */

  class DropdownPopupController : public SelectableListViewController<
                                      StandardMemoizedListViewDataSource> {
   public:
    DropdownPopupController(Responder* parentResponder,
                            ExplicitListViewDataSource* listDataSource,
                            Dropdown* dropdown,
                            DropdownCallback* callback = nullptr);

    // View Controller
    View* view() override { return &m_borderingView; }
    void didBecomeFirstResponder() override;
    bool handleEvent(Ion::Events::Event e) override;
    void close();

    // MemoizedListViewDataSource
    int numberOfRows() const override {
      return m_listViewDataSource->numberOfRows();
    }
    KDCoordinate defaultColumnWidth() override;
    int typeAtRow(int row) const override {
      return m_listViewDataSource->typeAtRow(row);
    }
    KDCoordinate nonMemoizedRowHeight(int row) override;
    int reusableCellCount(int type) override {
      return m_listViewDataSource->reusableCellCount(type);
    }
    PopupItemView* reusableCell(int index, int type) override;
    void fillCellForRow(HighlightCell* cell, int row) override;
    void resetSizeMemoization() override;

    HighlightCell* innerCellAtRow(int row);
    void resetMemoizationAndReload();
    void fillCell(int row);

   private:
    constexpr static int k_maxNumberOfPopupItems = 4;
    ExplicitListViewDataSource* m_listViewDataSource;
    PopupItemView m_popupViews[k_maxNumberOfPopupItems];
    KDCoordinate m_memoizedCellWidth;
    BorderingView m_borderingView;
    DropdownCallback* m_callback;
    Dropdown* m_dropdown;
  };

  DropdownPopupController m_popup;
};

}  // namespace Escher

#endif
