#ifndef ESCHER_EDITABLE_SEQUENCE_CELL_H
#define ESCHER_EDITABLE_SEQUENCE_CELL_H

#include <escher/editable_expression_model_cell.h>
#include <escher/layout_field.h>

#include "sequence_cell.h"

namespace Sequence {

class EditableSequenceCell : public AbstractSequenceCell {
 public:
  EditableSequenceCell(Escher::Responder* parentResponder,
                       Escher::InputEventHandlerDelegate* inputEventHandler,
                       Escher::LayoutFieldDelegate* layoutFieldDelegate)
      : AbstractSequenceCell(),
        m_expressionModelCell(parentResponder, inputEventHandler,
                              layoutFieldDelegate) {}
  Escher::EditableExpressionModelCell* expressionCell() override {
    return &m_expressionModelCell;
  }
  void updateSubviewsBackgroundAfterChangingState() override {
    m_expressionBackground = backgroundColor();
  }

  void layoutSubviews(bool force) override;

 private:
  static constexpr KDCoordinate k_expressionMargin = 5;
  Escher::EditableExpressionModelCell m_expressionModelCell;
};

}  // namespace Sequence

#endif
