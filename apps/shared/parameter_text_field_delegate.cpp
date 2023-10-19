#include "parameter_text_field_delegate.h"

#include <escher/text_field.h>

using namespace Escher;

namespace Shared {

bool ParameterTextFieldDelegate::textFieldDidReceiveEvent(
    AbstractTextField* textField, Ion::Events::Event event) {
  if (event == Ion::Events::Backspace && !textField->isEditing()) {
    textField->reinitTextBuffer();
    textField->setEditing(true);
    return true;
  }
  return MathTextFieldDelegate::textFieldDidReceiveEvent(textField, event);
}

}  // namespace Shared
