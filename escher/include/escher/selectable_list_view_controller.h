#ifndef ESCHER_SELECTABLE_LIST_VIEW_CONTROLLER_H
#define ESCHER_SELECTABLE_LIST_VIEW_CONTROLLER_H

#include <escher/explicit_list_view_data_source.h>
#include <escher/highlight_cell.h>
#include <escher/list_view_data_source.h>
#include <escher/responder.h>
#include <escher/selectable_list_view.h>
#include <escher/selectable_table_view.h>
#include <escher/selectable_table_view_delegate.h>
#include <escher/view_controller.h>

#include <type_traits>

namespace Escher {

class SelectableViewController : public ViewController,
                                 public SelectableListViewDataSource {
 public:
  SelectableViewController(Responder* parentResponder)
      : ViewController(parentResponder) {}

 protected:
  void centerTable(KDCoordinate availableHeight);
};

template <typename DataSource>
class SelectableListViewController : public SelectableViewController,
                                     public DataSource {
 public:
  SelectableListViewController(
      Responder* parentResponder,
      SelectableListViewDelegate* listDelegate = nullptr)
      : SelectableViewController(parentResponder),
        m_selectableListView(this, this, this, listDelegate) {}
  /* ViewController */
  View* view() override { return &m_selectableListView; }
  SelectableListView* selectableListView() { return &m_selectableListView; }

 protected:
  void didBecomeFirstResponder() override {
    Container::activeApp()->setFirstResponder(&m_selectableListView);
  }

  SelectableListView m_selectableListView;
};

class ExplicitSelectableListViewController
    : public SelectableListViewController<ExplicitListViewDataSource> {
 protected:
  using SelectableListViewController::SelectableListViewController;
  HighlightCell* selectedCell() { return cell(selectedRow()); }
};

/*
 * This Class is useful to create a SelectableListViewController of
 * the same type of cells with a constant number of rows that all have their own
 * reusable cell. The number of displayable cells should always be greater or
 * equal to the number of rows.
 */
template <typename Cell, int NumberOfCells, typename DataSource>
class SelectableCellListPage : public SelectableListViewController<DataSource> {
  static_assert(
      !std::is_same<DataSource, SimpleListViewDataSource>::value,
      "A SelectableCellListPage shouldn't use SimpleListViewDataSource as "
      "datasource because reusable cells are already handled here. Use a "
      "RegularListViewDataSource instead.");
  static_assert(NumberOfCells <= 5,
                "There should'nt be a need for more than 5 reusable cells.");

 public:
  constexpr static int k_numberOfCells = NumberOfCells;

  SelectableCellListPage(Responder* parent,
                         SelectableListViewDelegate* tableDelegate = nullptr)
      : SelectableListViewController<DataSource>(parent, tableDelegate) {}
  Cell* cellAtIndex(int i) {
    assert(i >= 0 && i < NumberOfCells);
    return &m_cells[i];
  }
  int numberOfRows() const override { return NumberOfCells; }
  int reusableCellCount(int type) override { return NumberOfCells; }
  HighlightCell* reusableCell(int i, int type) override {
    assert(type == 0);
    return cellAtIndex(i);
  }

 private:
  Cell m_cells[NumberOfCells];
};

}  // namespace Escher

#endif
