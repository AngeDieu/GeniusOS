#ifndef ESCHER_TEXT_FIELD_H
#define ESCHER_TEXT_FIELD_H

#include "abstract_text_field.h"

namespace Escher {

class TextField : public AbstractTextField {
 public:
  TextField(Responder* parentResponder, char* textBuffer, size_t textBufferSize,
            InputEventHandlerDelegate* inputEventHandlerDelegate,
            TextFieldDelegate* delegate = nullptr,
            KDFont::Size font = KDFont::Size::Large,
            float horizontalAlignment = KDContext::k_alignLeft,
            float verticalAlignment = KDContext::k_alignCenter,
            KDColor textColor = KDColorBlack,
            KDColor backgroundColor = KDColorWhite)
      : AbstractTextField(parentResponder, &m_contentView,
                          inputEventHandlerDelegate, delegate),
        m_contentView(textBuffer, textBufferSize, font, horizontalAlignment,
                      verticalAlignment, textColor, backgroundColor) {}

 protected:
  const AbstractTextField::ContentView* nonEditableContentView() const {
    return &m_contentView;
  }

  AbstractTextField::ContentView m_contentView;
};

}  // namespace Escher

#endif
