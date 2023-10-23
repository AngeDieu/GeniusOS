#include "values_controller.h"

#include <apps/constant.h>
#include <apps/shared/expression_display_permissions.h>
#include <apps/shared/poincare_helpers.h>
#include <assert.h>
#include <escher/clipboard.h>
#include <poincare/circuit_breaker_checkpoint.h>
#include <poincare/decimal.h>
#include <poincare/layout_helper.h>
#include <poincare/matrix_layout.h>
#include <poincare/serialization_helper.h>
#include <poincare/square_bracket_pair_layout.h>
#include <poincare/string_layout.h>

#include "../app.h"

using namespace Shared;
using namespace Poincare;
using namespace Escher;

namespace Graph {

/* PUBLIC */

ValuesController::ValuesController(
    Responder *parentResponder,
    Escher::InputEventHandlerDelegate *inputEventHandlerDelegate,
    ButtonRowController *header,
    FunctionParameterController *functionParameterController)
    : Shared::ValuesController(parentResponder, header),
      m_functionParameterController(functionParameterController),
      m_intervalParameterController(this, inputEventHandlerDelegate),
      m_derivativeParameterController(this),
      m_setIntervalButton(
          this, I18n::Message::IntervalSet,
          Invocation::Builder<ValuesController>(
              [](ValuesController *valuesController, void *sender) {
                StackViewController *stack =
                    ((StackViewController *)
                         valuesController->stackController());
                IntervalParameterSelectorController
                    *intervalSelectorController =
                        valuesController->intervalParameterSelectorController();
                if (intervalSelectorController->numberOfRows() == 1) {
                  IntervalParameterController *intervalController =
                      valuesController->intervalParameterController();
                  intervalController->setInterval(
                      valuesController->intervalAtColumn(0));
                  int i = 1;
                  intervalSelectorController->setStartEndMessages(
                      intervalController,
                      valuesController->symbolTypeAtColumn(&i));
                  stack->push(intervalController);
                  return true;
                }
                stack->push(intervalSelectorController);
                return true;
              },
              this),
          k_cellFont),
      m_exactValuesButton(
          this, I18n::Message::ExactResults,
          Invocation::Builder<ValuesController>(
              [](ValuesController *valuesController, void *sender) {
                valuesController->exactValuesButtonAction();
                return true;
              },
              this),
          &m_exactValuesDotView, k_cellFont),
      m_widthManager(this),
      m_heightManager(this),
      m_exactValuesAreActivated(false) {
  m_prefacedTwiceTableView.setPrefaceDelegate(this);
  initValueCells();
  m_exactValuesButton.setState(m_exactValuesAreActivated);
  setupSelectableTableViewAndCells(inputEventHandlerDelegate);
}

bool ValuesController::displayButtonExactValues() const {
  // When the store is full, the performances significantly drop.
  return !functionStore()->memoizationOverflows();
}

// ViewController

void ValuesController::viewDidDisappear() {
  activateExactValues(false);
  Shared::ValuesController::viewDidDisappear();
}

// TableViewDataSource

void ValuesController::fillCellForLocation(HighlightCell *cell, int column,
                                           int row) {
  // Handle hidden cells
  int type = typeAtLocation(column, row);
  if (type == k_notEditableValueCellType || type == k_editableValueCellType) {
    const int numberOfElementsInCol = numberOfElementsInColumn(column);
    EvenOddCell *eoCell = static_cast<EvenOddCell *>(cell);
    eoCell->setVisible(row <= numberOfElementsInCol + 1);
    if (row >= numberOfElementsInCol + 1) {
      static_cast<EvenOddCell *>(cell)->setEven(row % 2 == 0);
      if (type == k_notEditableValueCellType) {
        static_cast<EvenOddExpressionCell *>(eoCell)->setLayout(Layout());
      } else {
        assert(type == k_editableValueCellType);
        static_cast<Escher::AbstractEvenOddEditableTextCell *>(eoCell)
            ->editableTextCell()
            ->textField()
            ->setText("");
      }
      return;
    }
  }
  Shared::ValuesController::fillCellForLocation(cell, column, row);
}

int ValuesController::typeAtLocation(int column, int row) {
  symbolTypeAtColumn(&column);
  return Shared::ValuesController::typeAtLocation(column, row);
}

KDCoordinate ValuesController::separatorBeforeColumn(int column) {
  return column > 0 && typeAtLocation(column, 0) == k_abscissaTitleCellType
             ? Escher::Metric::TableSeparatorThickness
             : 0;
}

// ButtonRowDelegate

Escher::AbstractButtonCell *ValuesController::buttonAtIndex(
    int index, Escher::ButtonRowController::Position position) const {
  return index == 0 && displayButtonExactValues()
             ? const_cast<Escher::ButtonState *>(&m_exactValuesButton)
             : const_cast<Escher::AbstractButtonCell *>(&m_setIntervalButton);
}

// PrefacedTableViewDelegate

int ValuesController::columnToFreeze() {
  assert(numberOfColumns() > 0 && numberOfAbscissaColumns() > 0);
  if (selectedRow() == -1) {
    return -1;
  }
  int column = selectedColumn();
  while (typeAtLocation(column, 0) != k_abscissaTitleCellType) {
    column--;
  }
  assert(column >= 0);
  return column;
}

/* PRIVATE */

KDSize ValuesController::ApproximatedParametricCellSize() {
  KDSize layoutSize = SquareBracketPairLayoutNode::SizeGivenChildSize(KDSize(
      PrintFloat::glyphLengthForFloatWithPrecision(
          ::Preferences::VeryLargeNumberOfSignificantDigits) *
          KDFont::GlyphWidth(k_cellFont),
      2 * KDFont::GlyphHeight(k_cellFont) + GridLayoutNode::k_gridEntryMargin));
  return layoutSize +
         KDSize(Metric::SmallCellMargin * 2, Metric::SmallCellMargin * 2);
}

KDSize ValuesController::CellSizeWithLayout(Layout l) {
  EvenOddExpressionCell tempCell;
  tempCell.setFont(k_cellFont);
  tempCell.setLayout(l);
  return tempCell.minimalSizeForOptimalDisplay() +
         KDSize(Metric::SmallCellMargin * 2, Metric::SmallCellMargin * 2);
}

// TableViewDataSource

KDCoordinate ValuesController::nonMemoizedColumnWidth(int column) {
  KDCoordinate columnWidth;
  KDCoordinate maxColumnWidth = k_maxColumnWidth;
  int tempI = column;
  ContinuousFunctionProperties::SymbolType symbol = symbolTypeAtColumn(&tempI);
  if (tempI > 0 && symbol == ContinuousFunctionProperties::SymbolType::T) {
    // Default width is larger for parametric functions
    columnWidth = ApproximatedParametricCellSize().width();
  } else {
    columnWidth = Shared::ValuesController::defaultColumnWidth();
  }
  if (typeAtLocation(column, 0) == k_functionTitleCellType) {
    columnWidth = std::min(
        maxColumnWidth,
        std::max(CellSizeWithLayout(functionTitleLayout(column)).width(),
                 columnWidth));
  }
  if (!m_exactValuesAreActivated) {
    // Width is constant when displaying approximations
    return columnWidth;
  }
  int nRows = numberOfElementsInColumn(column) + 1;
  for (int row = 0; row < nRows; row++) {
    if (typeAtLocation(column, row) == k_notEditableValueCellType) {
      Layout l = memoizedLayoutForCell(column, row);
      assert(!l.isUninitialized());
      columnWidth = std::max(CellSizeWithLayout(l).width(), columnWidth);
      if (columnWidth > maxColumnWidth) {
        return maxColumnWidth;
      }
    }
  }
  return columnWidth;
}

KDCoordinate ValuesController::nonMemoizedRowHeight(int row) {
  KDCoordinate rowHeight = Shared::ValuesController::defaultRowHeight();
  KDCoordinate maxRowHeight = k_maxRowHeight;
  int nColumns = numberOfColumns();
  if (row == 0) {
    for (int i = 0; i < nColumns; i++) {
      if (typeAtLocation(i, 0) == k_functionTitleCellType) {
        rowHeight = std::max(
            CellSizeWithLayout(functionTitleLayout(i)).height(), rowHeight);
      }
      if (rowHeight > maxRowHeight) {
        return maxRowHeight;
      }
    }
    return rowHeight;
  }
  for (int i = 0; i < nColumns; i++) {
    int tempI = i;
    ContinuousFunctionProperties::SymbolType symbol =
        symbolTypeAtColumn(&tempI);
    if (!m_exactValuesAreActivated) {
      if (symbol != ContinuousFunctionProperties::SymbolType::T ||
          row >= numberOfElementsInColumn(i) + 1) {
        /* Height is constant when exact result is not displayed and
         * either there is no parametric function or it's the last row
         * of the column. */
        continue;
      } else {
        return ApproximatedParametricCellSize().height();
      }
    }
    if (typeAtLocation(i, row) == k_notEditableValueCellType &&
        row < numberOfElementsInColumn(i) + 1) {
      assert(m_exactValuesAreActivated);
      Layout l = memoizedLayoutForCell(i, row);
      assert(!l.isUninitialized());
      rowHeight = std::max(CellSizeWithLayout(l).height(), rowHeight);
      if (rowHeight > maxRowHeight) {
        return maxRowHeight;
      }
    }
  }
  return rowHeight;
}

// ColumnHelper

int ValuesController::fillColumnName(int column, char *buffer) {
  if (typeAtLocation(column, 0) == k_functionTitleCellType) {
    Layout functionTitle = functionTitleLayout(column, true);
    constexpr size_t bufferNameSize =
        ContinuousFunction::k_maxNameWithArgumentSize + 1;
    int size = functionTitle.serializeForParsing(buffer, bufferNameSize);
    // Serialization may have introduced system parentheses.
    SerializationHelper::ReplaceSystemParenthesesAndBracesByUserParentheses(
        buffer, bufferNameSize - 1);
    return size;
  }
  return Shared::ValuesController::fillColumnName(column, buffer);
}

// EditableCellTableViewController

void ValuesController::reloadEditedCell(int column, int row) {
  if (m_exactValuesAreActivated) {
    // Sizes might have changed
    selectableTableView()->reloadData();
    return;
  }
  Shared::ValuesController::reloadEditedCell(column, row);
}

void ValuesController::setTitleCellStyle(HighlightCell *cell, int column) {
  if (typeAtLocation(column, 0) == k_abscissaTitleCellType) {
    return;
  }
  Shared::ValuesController::setTitleCellStyle(cell, column);
}

// Shared::ValuesController

Ion::Storage::Record ValuesController::recordAtColumn(int i) {
  bool isDerivative = false;
  return recordAtColumn(i, &isDerivative);
}

void ValuesController::updateNumberOfColumns() const {
  for (size_t symbolTypeIndex = 0; symbolTypeIndex < k_maxNumberOfSymbolTypes;
       symbolTypeIndex++) {
    m_numberOfValuesColumnsForType[symbolTypeIndex] = 0;
  }
  int numberOfActiveFunctionsInTable =
      functionStore()->numberOfActiveFunctionsInTable();
  for (int i = 0; i < numberOfActiveFunctionsInTable; i++) {
    Ion::Storage::Record record =
        functionStore()->activeRecordInTableAtIndex(i);
    ExpiringPointer<ContinuousFunction> f =
        functionStore()->modelForRecord(record);
    int symbolTypeIndex = static_cast<int>(f->properties().symbolType());
    m_numberOfValuesColumnsForType[symbolTypeIndex] +=
        numberOfColumnsForRecord(record);
  }
  m_numberOfColumns = 0;
  for (size_t symbolTypeIndex = 0; symbolTypeIndex < k_maxNumberOfSymbolTypes;
       symbolTypeIndex++) {
    // Count abscissa column if the sub table does exist
    m_numberOfColumns += numberOfColumnsForSymbolType(symbolTypeIndex);
  }
}

Poincare::Layout *ValuesController::memoizedLayoutAtIndex(int i) {
  assert(i >= 0 && i < k_maxNumberOfDisplayableCells);
  return &m_memoizedLayouts[i];
}

Layout ValuesController::functionTitleLayout(int column,
                                             bool forceShortVersion) {
  assert(typeAtLocation(column, 0) == k_functionTitleCellType);
  bool isDerivative = false;
  Ion::Storage::Record record = recordAtColumn(column, &isDerivative);
  Shared::ExpiringPointer<ContinuousFunction> function =
      functionStore()->modelForRecord(record);
  if (isDerivative) {
    return function->derivativeTitleLayout();
  }
  return function->titleLayout(textFieldDelegateApp()->localContext(),
                               forceShortVersion || function->isNamed());
}

int ValuesController::numberOfAbscissaColumnsBeforeAbsoluteColumn(
    int column) const {
  int abscissaColumns = 0;
  int symbolType = column < 0 ? k_maxNumberOfSymbolTypes
                              : (int)symbolTypeAtColumn(&column) + 1;
  for (int symbolTypeIndex = 0; symbolTypeIndex < symbolType;
       symbolTypeIndex++) {
    abscissaColumns += (m_numberOfValuesColumnsForType[symbolTypeIndex] > 0);
  }
  return abscissaColumns;
}

int ValuesController::numberOfAbscissaColumnsBeforeValuesColumn(
    int column) const {
  int abscissaColumns = 0;
  int valuesColumns = 0;
  size_t symbolTypeIndex = 0;
  while (valuesColumns <= column) {
    assert(symbolTypeIndex < k_maxNumberOfSymbolTypes);
    const int numberOfValuesColumnsForType =
        m_numberOfValuesColumnsForType[symbolTypeIndex++];
    valuesColumns += numberOfValuesColumnsForType;
    abscissaColumns += (numberOfValuesColumnsForType > 0);
  }
  return abscissaColumns;
}

void ValuesController::setStartEndMessages(
    Shared::IntervalParameterController *controller, int column) {
  m_intervalParameterSelectorController.setStartEndMessages(
      controller, symbolTypeAtColumn(&column));
}

void ValuesController::createMemoizedLayout(int column, int row, int index) {
  double abscissa;
  bool isDerivative = false;
  Shared::ExpiringPointer<ContinuousFunction> function =
      functionAtIndex(column, row, &abscissa, &isDerivative);
  Poincare::Context *context = textFieldDelegateApp()->localContext();
  Expression result;
  if (isDerivative) {
    // Compute derivative approximate result
    result = Float<double>::Builder(
        function->approximateDerivative(abscissa, context, 0, false));
  } else {
    // Compute exact result
    result = function->expressionReduced(context);
    Poincare::VariableContext abscissaContext =
        Poincare::VariableContext(Shared::Function::k_unknownName, context);
    Poincare::Expression abscissaExpression =
        Poincare::Decimal::Builder<double>(abscissa);
    abscissaContext.setExpressionForSymbolAbstract(
        abscissaExpression,
        Symbol::Builder(Shared::Function::k_unknownName,
                        strlen(Shared::Function::k_unknownName)));
    bool simplificationFailure = false;
    PoincareHelpers::CloneAndSimplify(
        &result, &abscissaContext, Poincare::ReductionTarget::User,
        Poincare::SymbolicComputation::
            ReplaceAllSymbolsWithDefinitionsOrUndefined,
        Poincare::UnitConversion::Default,
        Poincare::Preferences::sharedPreferences, true, &simplificationFailure);
    /* Approximate in case of simplification failure, as we cannot display a
     * non-beautified expression. */
    Expression approximation =
        PoincareHelpers::Approximate<double>(result, context);
    if (simplificationFailure || !m_exactValuesAreActivated ||
        ExpressionDisplayPermissions::ShouldOnlyDisplayApproximation(
            function->originalEquation(), result, approximation, context)) {
      // Do not show exact expressions in certain cases, use approximate result
      result = approximation;
    }
  }
  *memoizedLayoutAtIndex(index) = result.createLayout(
      Preferences::PrintFloatMode::Decimal,
      Preferences::VeryLargeNumberOfSignificantDigits, context);
}

int ValuesController::numberOfColumnsForAbscissaColumn(int column) {
  return numberOfColumnsForSymbolType((int)symbolTypeAtColumn(&column));
}

void ValuesController::updateSizeMemoizationForColumnAfterIndexChanged(
    int column, KDCoordinate columnPreviousWidth, int row) {
  // Update the size only if column becomes larger
  if (m_exactValuesAreActivated) {
    KDCoordinate minimalWidthForColumn = std::min(
        k_maxColumnWidth,
        CellSizeWithLayout(memoizedLayoutForCell(column, row)).width());
    if (columnPreviousWidth < minimalWidthForColumn) {
      m_widthManager.updateMemoizationForIndex(column, columnPreviousWidth,
                                               minimalWidthForColumn);
    }
  }
}

Shared::Interval *ValuesController::intervalAtColumn(int column) {
  return App::app()->intervalForSymbolType(symbolTypeAtColumn(&column));
}

I18n::Message ValuesController::valuesParameterMessageAtColumn(
    int column) const {
  return ContinuousFunctionProperties::MessageForSymbolType(
      symbolTypeAtColumn(&column));
}

Shared::ExpressionFunctionTitleCell *ValuesController::functionTitleCells(
    int j) {
  assert(j >= 0 && j < k_maxNumberOfDisplayableColumns);
  return &m_functionTitleCells[j];
}

EvenOddExpressionCell *ValuesController::valueCells(int j) {
  assert(j >= 0 && j < k_maxNumberOfDisplayableCells);
  return &m_valueCells[j];
}

Escher::AbstractEvenOddEditableTextCell *ValuesController::abscissaCells(
    int j) {
  assert(j >= 0 && j < k_maxNumberOfDisplayableAbscissaCells);
  return &m_abscissaCells[j];
}

Escher::EvenOddMessageTextCell *ValuesController::abscissaTitleCells(int j) {
  assert(j >= 0 && j < abscissaTitleCellsCount());
  return &m_abscissaTitleCells[j];
}

// Graph::ValuesController

template <class T>
T *ValuesController::parameterController() {
  bool isDerivative = false;
  Ion::Storage::Record record = recordAtColumn(selectedColumn(), &isDerivative);
  if (!functionStore()->modelForRecord(record)->properties().isCartesian()) {
    return nullptr;
  }
  if (isDerivative) {
    m_derivativeParameterController.setRecord(record);
    return &m_derivativeParameterController;
  }
  m_functionParameterController->setRecord(record);
  return m_functionParameterController;
}

bool ValuesController::exactValuesButtonAction() {
  assert(m_exactValuesButton.state() == m_exactValuesAreActivated);
  /* Approximated values computation is much faster than exact values so it's
   * uninterruptable and if the exact values computation is interrupted, it
   * falls back on approximated values computation. */
  resetLayoutMemoization();

  if (m_exactValuesAreActivated) {
    activateExactValues(false);
    resetLayoutMemoization();
    m_selectableTableView.reloadData();
    return true;
  }

  {
    CircuitBreakerCheckpoint checkpoint(
        Ion::CircuitBreaker::CheckpointType::Back);
    if (CircuitBreakerRun(checkpoint)) {
      activateExactValues(true);
      m_selectableTableView.reloadData();
      return true;
    } else {
      activateExactValues(false);
      resetLayoutMemoization();
      m_selectableTableView.reloadData();
      return false;
    }
  }
}

void ValuesController::activateExactValues(bool activate) {
  m_exactValuesAreActivated = activate;
  m_exactValuesButton.setState(m_exactValuesAreActivated);
}

Ion::Storage::Record ValuesController::recordAtColumn(int i,
                                                      bool *isDerivative) {
  assert(typeAtLocation(i, 0) == k_functionTitleCellType);
  ContinuousFunctionProperties::SymbolType symbolType = symbolTypeAtColumn(&i);
  int index = 1;
  int numberOfActiveFunctionsInTableOfSymbolType =
      functionStore()->numberOfActiveFunctionsInTableOfSymbolType(symbolType);
  for (int k = 0; k < numberOfActiveFunctionsInTableOfSymbolType; k++) {
    Ion::Storage::Record record =
        functionStore()->activeRecordOfSymbolTypeInTableAtIndex(symbolType, k);
    const int numberOfColumnsForCurrentRecord =
        numberOfColumnsForRecord(record);
    if (index <= i && i < index + numberOfColumnsForCurrentRecord) {
      ExpiringPointer<ContinuousFunction> f =
          functionStore()->modelForRecord(record);
      *isDerivative = i != index && f->canDisplayDerivative();
      return record;
    }
    index += numberOfColumnsForCurrentRecord;
  }
  assert(false);
  return nullptr;
}

Shared::ExpiringPointer<ContinuousFunction> ValuesController::functionAtIndex(
    int column, int row, double *abscissa, bool *isDerivative) {
  *abscissa = intervalAtColumn(column)->element(
      row - 1);  // Subtract the title row from row to get the element index
  Ion::Storage::Record record = recordAtColumn(column, isDerivative);
  return functionStore()->modelForRecord(record);
}

int ValuesController::numberOfColumnsForRecord(
    Ion::Storage::Record record) const {
  ExpiringPointer<ContinuousFunction> f =
      functionStore()->modelForRecord(record);
  return 1 + (f->properties().isCartesian() && f->displayDerivative());
}

int ValuesController::numberOfColumnsForSymbolType(int symbolTypeIndex) const {
  return m_numberOfValuesColumnsForType[symbolTypeIndex] +
         (m_numberOfValuesColumnsForType[symbolTypeIndex] >
          0);  // Count abscissa column if there is one
}

ContinuousFunctionProperties::SymbolType ValuesController::symbolTypeAtColumn(
    int *column) const {
  // column becomes the index of the column in the sub table (of symbol type)
  size_t symbolTypeIndex = 0;
  while (*column >= numberOfColumnsForSymbolType(symbolTypeIndex)) {
    *column -= numberOfColumnsForSymbolType(symbolTypeIndex++);
    assert(symbolTypeIndex < k_maxNumberOfSymbolTypes);
  }
  return static_cast<ContinuousFunctionProperties::SymbolType>(symbolTypeIndex);
}

}  // namespace Graph
