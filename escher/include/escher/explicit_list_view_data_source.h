#ifndef ESCHER_EXPLICIT_LIST_VIEW_DATA_SOURCE_H
#define ESCHER_EXPLICIT_LIST_VIEW_DATA_SOURCE_H

#include <escher/list_view_data_source.h>
#include <escher/table_size_1D_manager.h>

namespace Escher {

/* This class is a ListViewDataSource where each cell has its own type. It
 * basically means that the reusableCell mecanism is erased which results in
 * easier manipulation of heterogeneous cells but should not be used in list
 * with a long or dynamic cell count. */
class ExplicitListViewDataSource : public ListViewDataSource {
 public:
  ExplicitListViewDataSource() : ListViewDataSource(), m_heightManager(this) {}

  int typeAtRow(int row) const override final { return row; }
  int reusableCellCount(int type) override final { return 1; }
  HighlightCell* reusableCell(int index, int type) override final {
    return cell(type);
  }
  /* Cells are not reusable: they are filled only directly in constructor and
   * viewWillAppear or other methods than can modify them like handleEvent. */
  void fillCellForRow(Escher::HighlightCell* cell, int row) override final {}

 protected:
  virtual HighlightCell* cell(int index) = 0;
  // TODO: Make this method final (take the example of DropdownPopupController)
  KDCoordinate nonMemoizedRowHeight(int row) override {
    return protectedNonMemoizedRowHeight(cell(row), row);
  }

 private:
  /* This size manager leverages the fact that each cell object
   * is only used by one explicit cell. Once the cell has been filled with its
   * content, it's not very costly to call minimalSizeForOptimalDisplay. */
  class ExplicitListRowHeightManager : public TableSize1DManager {
   public:
    ExplicitListRowHeightManager(ExplicitListViewDataSource* dataSource)
        : m_dataSource(dataSource), m_sizesAreComputed(false) {}
    KDCoordinate computeSizeAtIndex(int i) override;
    KDCoordinate computeCumulatedSizeBeforeIndex(
        int i, KDCoordinate defaultSize) override {
      return k_undefinedSize;
    }
    int computeIndexAfterCumulatedSize(KDCoordinate offset,
                                       KDCoordinate defaultSize) override {
      return k_undefinedSize;
    }

    void resetSizeMemoization(bool force) override {
      m_sizesAreComputed = false;
    }

   private:
    ExplicitListViewDataSource* m_dataSource;
    bool m_sizesAreComputed;
  };

  bool canSelectCellAtRow(int row) override;

  TableSize1DManager* rowHeightManager() override { return &m_heightManager; }

  ExplicitListRowHeightManager m_heightManager;
};

}  // namespace Escher

#endif
