#include "list_controller.h"

#include <apps/i18n.h>
#include <assert.h>
#include <escher/metric.h>
#include <poincare/code_point_layout.h>
#include <poincare/matrix_layout.h>
#include <poincare/symbol_abstract.h>

#include "../app.h"

using namespace Shared;
using namespace Escher;
using namespace Poincare;

namespace Graph {

ListController::ListController(
    Responder *parentResponder, ButtonRowController *header,
    ButtonRowController *footer,
    FunctionParameterController *functionParameterController)
    : Shared::FunctionListController(parentResponder, header, footer,
                                     I18n::Message::AddFunction),
      m_editableCell(this, this),
      m_parameterController(functionParameterController),
      m_modelsParameterController(this, this),
      m_modelsStackController(nullptr, &m_modelsParameterController,
                              StackViewController::Style::PurpleWhite),
      m_parameterColumnSelected(false) {
  static_assert(k_newModelMargin == FunctionCell::k_colorIndicatorThickness +
                                        Metric::BigCellMargin);
}

/* TableViewDataSource */

HighlightCell *ListController::reusableCell(int index, int type) {
  assert(index >= 0 && index < reusableCellCount(type));
  if (type == k_editableCellType) {
    return &m_editableCell;
  }
  if (type == k_addNewModelCellType) {
    return &m_addNewModelCell;
  }
  assert(type == k_expressionCellType);
  return functionCells(index);
}

int ListController::reusableCellCount(int type) {
  if (type == k_expressionCellType) {
    return maxNumberOfDisplayableRows();
  }
  return 1;
}

/* ViewController */

void ListController::viewWillAppear() {
  // A function could have been deleted from the option menu of the Graph tab.
  resetMemoization();
  Shared::FunctionListController::viewWillAppear();
  /* FunctionListcontroller::didEnterResponderChain might not be called,
   * (if the list tab is displayed but not selected using Back-Back)
   * therefore we also need to manually reload the table here. */
  selectableListView()->reloadData(false);
}

// Fills buffer with a default function equation, such as "f(x)=", "y=" or "r="
void ListController::fillWithDefaultFunctionEquation(
    char *buffer, size_t bufferSize,
    FunctionModelsParameterController *modelsParameterController,
    CodePoint symbol) const {
  size_t length;
  if (symbol == ContinuousFunction::k_polarSymbol) {
    length = UTF8Decoder::CodePointToChars(ContinuousFunction::k_radiusSymbol,
                                           buffer, bufferSize);
  } else if (FunctionModelsParameterController::EquationsPrefered()) {
    length = UTF8Decoder::CodePointToChars(ContinuousFunction::k_ordinateSymbol,
                                           buffer, bufferSize);
  } else {
    length = modelsParameterController->DefaultName(buffer, bufferSize);
    assert(bufferSize > length);
    buffer[length++] = '(';
    length += UTF8Decoder::CodePointToChars(symbol, buffer + length,
                                            bufferSize - length);
    assert(bufferSize > length + 2);
    buffer[length++] = ')';
  }
  buffer[length++] = '=';
  buffer[length++] = 0;
}

// Return true if given layout contains θ
bool ListController::layoutRepresentsPolarFunction(Layout l) const {
  Layout match = l.recursivelyMatches([](Layout layout) {
    return layout.type() == LayoutNode::Type::CodePointLayout &&
                   static_cast<CodePointLayout &>(layout).codePoint() ==
                       ContinuousFunction::k_polarSymbol
               ? TrinaryBoolean::True
               : TrinaryBoolean::Unknown;
  });
  return !match.isUninitialized();
}

/* Return true if given layout contains a Matrix of correct dimensions as first
 * child. Note: A more precise detection would require an expression reduction
 * to distinguish:
 * - sum(1,t,0,100) : Cartesian
 * - norm([[4][5]]) : Cartesian
 * - 31*[[4][5]]*10 : Parametric */
bool ListController::layoutRepresentsParametricFunction(Layout l) const {
  if (l.type() == LayoutNode::Type::HorizontalLayout &&
      l.numberOfChildren() > 0) {
    l = l.childAtIndex(0);
  }
  if (l.type() != LayoutNode::Type::MatrixLayout) {
    return false;
  }
  MatrixLayout m = static_cast<MatrixLayout &>(l);
  return m.numberOfColumns() == 1 && m.numberOfRows() == 2;
}

bool ListController::completeEquation(InputEventHandler *equationField,
                                      CodePoint symbol) {
  // Retrieve the edited function
  ExpiringPointer<ContinuousFunction> f =
      modelStore()->modelForRecord(modelStore()->recordAtIndex(selectedRow()));
  if (f->isNull() || f->properties().status() ==
                         ContinuousFunctionProperties::Status::Undefined) {
    // Function is new or undefined, complete the equation with a default name
    constexpr size_t k_bufferSize =
        Shared::ContinuousFunction::k_maxDefaultNameSize + sizeof("(x)=") - 1;
    char buffer[k_bufferSize];
    // Insert "f(x)=", with f the default function name and x the symbol
    fillWithDefaultFunctionEquation(buffer, k_bufferSize,
                                    &m_modelsParameterController, symbol);
    return equationField->handleEventWithText(buffer, false, true);
  }
  // Insert the name, symbol and equation symbol of the existing function
  constexpr size_t k_bufferSize =
      SymbolAbstractNode::k_maxNameSize + sizeof("(x)≥") - 1;
  static_assert(k_bufferSize >= sizeof("r="),
                "k_bufferSize should fit both situations.");
  char buffer[k_bufferSize];
  size_t nameLength = f->nameWithArgument(buffer, k_bufferSize);
  nameLength += strlcpy(buffer + nameLength, f->properties().equationSymbol(),
                        k_bufferSize - nameLength);
  assert(nameLength < k_bufferSize);
  return equationField->handleEventWithText(buffer, false, true);
}

// TODO: factorize with solver
bool ListController::layoutFieldDidReceiveEvent(LayoutField *layoutField,
                                                Ion::Events::Event event) {
  m_parameterColumnSelected = false;
  if (layoutField->isEditing() && layoutField->shouldFinishEditing(event) &&
      !layoutField->layout().hasTopLevelEquationSymbol()) {
    char buffer[TextField::MaxBufferSize()];
    layoutField->layout().serializeForParsing(buffer,
                                              TextField::MaxBufferSize());
    Expression parsedExpression = Expression::Parse(buffer, nullptr);
    if (parsedExpression.isUninitialized() ||
        (!parsedExpression.isOfType({
             ExpressionNode::Type::Comparison,
             ExpressionNode::Type::Point,
         }) &&
         !parsedExpression.deepIsList(nullptr))) {
      layoutField->putCursorOnOneSide(OMG::Direction::Left());
      CodePoint symbol =
          layoutRepresentsPolarFunction(layoutField->layout())
              ? ContinuousFunction::k_polarSymbol
              : (layoutRepresentsParametricFunction(layoutField->layout())
                     ? ContinuousFunction::k_parametricSymbol
                     : ContinuousFunction::k_cartesianSymbol);
      if (!completeEquation(layoutField, symbol)) {
        layoutField->putCursorOnOneSide(OMG::Direction::Right());
        Container::activeApp()->displayWarning(I18n::Message::RequireEquation);
        return true;
      }
    }
  }
  return MathFieldDelegate::layoutFieldDidReceiveEvent(layoutField, event);
}

CodePoint ListController::XNT() {
  int selectedFunctionIndex = selectedRow();
  if (!App::app()->isStoreMenuOpen() && selectedFunctionIndex >= 0) {
    assert(selectedFunctionIndex < modelStore()->numberOfModels());
    Ion::Storage::Record record =
        modelStore()->recordAtIndex(selectedFunctionIndex);
    return modelStore()->modelForRecord(record)->symbol();
  }
  return ContinuousFunction::k_cartesianSymbol;
}

void ListController::editExpression(Ion::Events::Event event) {
  ExpressionModelListController::editExpression(event);
  m_editableCell.setHighlighted(true);
}

/* Responder */

KDCoordinate ListController::expressionRowHeight(int j) {
  if (j == m_editedCellIndex) {
    return m_editableCell.minimalSizeForOptimalDisplay().height();
  }
  if (typeAtRow(j) == k_addNewModelCellType) {
    return Shared::FunctionListController::expressionRowHeight(j);
  }
  FunctionCell tempCell;
  fillCellForRow(&tempCell, j);
  return tempCell.minimalSizeForOptimalDisplay().height();
}

bool ListController::handleEvent(Ion::Events::Event event) {
  // Here we handle an additional parameter column, within FunctionCell's button
  if (selectedRow() >= 0 && selectedRow() <= numberOfRows() &&
      !isAddEmptyRow(selectedRow())) {
    // Selected row is a function cell
    if (m_parameterColumnSelected) {
      // Parameter column is selected
      if (event == Ion::Events::OK || event == Ion::Events::EXE) {
        // Open function parameter menu
        m_parameterController->setUseColumnTitle(false);
        configureFunction(
            modelStore()->recordAtIndex(modelIndexForRow(selectedRow())));
        return true;
      }
      if (event == Ion::Events::Left) {
        // Leave parameter column
        m_parameterColumnSelected = false;
        selectableListView()->reloadData();
        return true;
      }
    } else if (event == Ion::Events::Right) {
      // Enter parameter column
      m_parameterColumnSelected = true;
      selectableListView()->reloadData();
      return true;
    }
  }
  if (event == Ion::Events::Up) {
    if (selectedRow() == -1) {
      footer()->setSelectedButton(-1);
      selectableListView()->selectCell(numberOfRows() - 1);
      Container::activeApp()->setFirstResponder(selectableListView());
      return true;
    }
    selectableListView()->deselectTable();
    assert(selectedRow() == -1);
    tabController()->selectTab();
    return true;
  }
  if (selectedRow() < 0) {
    return false;
  }
  if (event == Ion::Events::Down) {
    selectableListView()->deselectTable();
    footer()->setSelectedButton(0);
    return true;
  }
  return handleEventOnExpression(event);
}

Shared::ListParameterController *ListController::parameterController() {
  return m_parameterController;
}

int ListController::maxNumberOfDisplayableRows() {
  return k_maxNumberOfDisplayableRows;
}

HighlightCell *ListController::functionCells(int row) {
  assert(row >= 0 && row < k_maxNumberOfDisplayableRows);
  return &m_expressionCells[row];
}

void ListController::fillCellForRow(HighlightCell *cell, int row) {
  assert(cell != nullptr);
  EvenOddCell *evenOddCell = static_cast<EvenOddCell *>(cell);
  evenOddCell->setEven(row % 2 == 0);
  int type = typeAtRow(row);
  if (type == k_addNewModelCellType) {
    evenOddCell->reloadCell();
    return;
  }
  assert(type == k_expressionCellType || type == k_editableCellType);
  AbstractFunctionCell *functionCell =
      static_cast<AbstractFunctionCell *>(cell);
  ExpiringPointer<ContinuousFunction> f =
      modelStore()->modelForRecord(modelStore()->recordAtIndex(row));
  if (type == k_expressionCellType) {
    functionCell->setLayout(f->layout());
    functionCell->setMessage(
        Preferences::sharedPreferences->examMode().forbidImplicitPlots()
            ? I18n::Message::Default
            : f->properties().caption());
    KDColor textColor = f->isActive() ? KDColorBlack : Palette::GrayDark;
    functionCell->setTextColor(textColor);
    static_cast<FunctionCell *>(functionCell)
        ->setParameterSelected(m_parameterColumnSelected);
  }
  KDColor functionColor = f->isActive() ? f->color() : Palette::GrayDark;
  functionCell->setColor(functionColor);
  functionCell->reloadCell();
}

/* TODO: Restore behaviour
FunctionToolbox *ListController::toolbox() {
  return App::app()->functionToolbox();
}*/

void ListController::addModel() {
  Container::activeApp()->displayModalViewController(
      &m_modelsStackController, 0.f, 0.f, Metric::PopUpTopMargin,
      Metric::PopUpRightMargin, 0, Metric::PopUpLeftMargin);
}

ContinuousFunctionStore *ListController::modelStore() const {
  return App::app()->functionStore();
}

}  // namespace Graph
