#ifndef ESCHER_EDITABLE_SEQUENCE_CELL_H
#define ESCHER_EDITABLE_SEQUENCE_CELL_H

#include <escher/editable_expression_model_cell.h>

#include "sequence_cell.h"

namespace Sequence {

class EditableSequenceCell
    : public TemplatedSequenceCell<Shared::WithEditableExpressionCell> {
 public:
  EditableSequenceCell(Escher::Responder* parentResponder,
                       Escher::LayoutFieldDelegate* layoutFieldDelegate);

  void updateSubviewsBackgroundAfterChangingState() override {
    m_expressionBackground = backgroundColor();
  }

  void layoutSubviews(bool force) override;

 private:
  static constexpr KDCoordinate k_expressionMargin = 5;
};

}  // namespace Sequence

#endif
