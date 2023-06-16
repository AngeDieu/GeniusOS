#ifndef ESCHER_TABLE_SIZE_1D_MANAGER_H
#define ESCHER_TABLE_SIZE_1D_MANAGER_H

#include <kandinsky/coordinate.h>

namespace Escher {

class TableViewDataSource;

/* This class and its children speed up the computation of width, height,
 * cumulatedSize and indexAfterCumulatedSize in TableViewDataSource */
class TableSize1DManager {
 public:
  constexpr static KDCoordinate k_undefinedSize = -1;
  // The sizes are memoized and returned with separators
  virtual KDCoordinate computeSizeAtIndex(int i) = 0;
  virtual KDCoordinate computeCumulatedSizeBeforeIndex(
      int i, KDCoordinate defaultSize) = 0;
  virtual int computeIndexAfterCumulatedSize(KDCoordinate offset,
                                             KDCoordinate defaultSize) = 0;
  virtual bool sizeAtIndexIsMemoized(int i) const { return false; }

  virtual void resetMemoization(bool force = true) {}
  virtual void lockMemoization(bool state) const {}
};

/* Use RegularTableSize1DManager if the height or width is constant.
 * The main advantage of this class is to compute cumulatedSizeAtIndex
 * and indexAfterCumulatedSize without going through a for-loop and adding
 * n-times for the same value.
 *
 * WARNING: This size manager does not work properly with table separators. */
class RegularTableSize1DManager : public TableSize1DManager {
 public:
  KDCoordinate computeSizeAtIndex(int i) override { return k_undefinedSize; }
  KDCoordinate computeCumulatedSizeBeforeIndex(
      int i, KDCoordinate defaultSize) override {
    return defaultSize == k_undefinedSize ? k_undefinedSize : i * defaultSize;
  }
  int computeIndexAfterCumulatedSize(KDCoordinate offset,
                                     KDCoordinate defaultSize) override {
    return (defaultSize == k_undefinedSize || defaultSize == 0)
               ? defaultSize
               : offset / defaultSize;
  }
};

/* MemoizedTableSize1DManager are used for table which have a dynamically
 * variable height or width.
 * This property slows down navigation due to complex cell size calculation.
 * To avoid that, cells size and cumulated size is memoized around the most
 * recently used cells. Total size is also memoized. */
class MemoizedTableSize1DManager : public TableSize1DManager {
 public:
  MemoizedTableSize1DManager(TableViewDataSource* tableViewDataSource)
      : m_dataSource(tableViewDataSource), m_memoizationLockedLevel(0) {}
  KDCoordinate computeSizeAtIndex(int i) override;
  KDCoordinate computeCumulatedSizeBeforeIndex(
      int i, KDCoordinate defaultSize) override;
  int computeIndexAfterCumulatedSize(KDCoordinate offset,
                                     KDCoordinate defaultSize) override;

  void resetMemoization(bool force = true) override;
  void lockMemoization(bool state) const override;

  bool sizeAtIndexIsMemoized(int i) const override {
    return i >= m_memoizedIndexOffset &&
           i < m_memoizedIndexOffset + memoizedLinesCount();
  }

  void updateMemoizationForIndex(int index, KDCoordinate previousSize,
                                 KDCoordinate newSize = k_undefinedSize);
  void deleteIndexFromMemoization(int index, KDCoordinate previousSize);

 protected:
  virtual int numberOfLines() const = 0;  // Return number of rows or columns
  virtual KDCoordinate sizeAtIndex(int i) const = 0;
  virtual KDCoordinate nonMemoizedSizeAtIndex(int i) const = 0;
  virtual KDCoordinate nonMemoizedCumulatedSizeBeforeIndex(int i) const = 0;
  virtual int memoizedLinesCount() const = 0;
  virtual KDCoordinate* memoizedSizes() = 0;
  TableViewDataSource* m_dataSource;

 private:
  int getMemoizedIndex(int index) const;
  void setMemoizationIndex(int index);
  void shiftMemoization(bool lowerIndex);
  KDCoordinate m_memoizedCumulatedSizeOffset;
  KDCoordinate m_memoizedTotalSize;
  int m_memoizedIndexOffset;
  mutable int m_memoizationLockedLevel;
};

template <int N>
class TemplatedMemoizedTableSize1DManager : public MemoizedTableSize1DManager {
 public:
  TemplatedMemoizedTableSize1DManager(TableViewDataSource* tableViewDataSource)
      : MemoizedTableSize1DManager(tableViewDataSource) {
    // Must be done here since virtual functions are not virtual in constructors
    resetMemoization(true);
  }

 private:
  constexpr static int k_memoizedLinesCount = N;
  KDCoordinate* memoizedSizes() override { return m_memoizedSizes; }
  int memoizedLinesCount() const override { return k_memoizedLinesCount; }
  KDCoordinate m_memoizedSizes[k_memoizedLinesCount];
};

template <int N>
class MemoizedColumnWidthManager
    : public TemplatedMemoizedTableSize1DManager<N> {
 public:
  MemoizedColumnWidthManager(TableViewDataSource* dataSource)
      : TemplatedMemoizedTableSize1DManager<N>(dataSource) {}

 protected:
  int numberOfLines() const override {
    return this->m_dataSource->numberOfColumns();
  }
  KDCoordinate sizeAtIndex(int i) const override {
    return this->m_dataSource->columnWidth(i);
  }
  KDCoordinate nonMemoizedSizeAtIndex(int i) const override {
    return this->m_dataSource->nonMemoizedColumnWidth(i, true);
  }
  KDCoordinate nonMemoizedCumulatedSizeBeforeIndex(int i) const override {
    return this->m_dataSource->nonMemoizedCumulatedWidthBeforeColumn(i);
  }
};

template <int N>
class MemoizedRowHeightManager : public TemplatedMemoizedTableSize1DManager<N> {
 public:
  MemoizedRowHeightManager(TableViewDataSource* dataSource)
      : TemplatedMemoizedTableSize1DManager<N>(dataSource) {}

 protected:
  int numberOfLines() const override {
    return this->m_dataSource->numberOfRows();
  }
  KDCoordinate sizeAtIndex(int i) const override {
    return this->m_dataSource->rowHeight(i);
  }
  KDCoordinate nonMemoizedSizeAtIndex(int i) const override {
    return this->m_dataSource->nonMemoizedRowHeight(i, true);
  }
  KDCoordinate nonMemoizedCumulatedSizeBeforeIndex(int i) const override {
    return this->m_dataSource->nonMemoizedCumulatedHeightBeforeRow(i);
  }
};

}  // namespace Escher
#endif
