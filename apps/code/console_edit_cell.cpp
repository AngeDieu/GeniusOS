#include "console_edit_cell.h"

#include <apps/global_preferences.h>
#include <apps/i18n.h>
#include <assert.h>
#include <escher/app.h>

#include <algorithm>

#include "console_controller.h"

using namespace Escher;

namespace Code {

ConsoleEditCell::ConsoleEditCell(
    Responder *parentResponder,
    InputEventHandlerDelegate *inputEventHandlerDelegate,
    TextFieldDelegate *delegate)
    : HighlightCell(),
      Responder(parentResponder),
      m_promptView(GlobalPreferences::sharedGlobalPreferences->font(), nullptr,
                   KDContext::k_alignLeft, KDContext::k_alignCenter),
      m_textField(this, nullptr, TextField::MaxBufferSize(),
                  inputEventHandlerDelegate, delegate,
                  GlobalPreferences::sharedGlobalPreferences->font()) {}

int ConsoleEditCell::numberOfSubviews() const { return 2; }

View *ConsoleEditCell::subviewAtIndex(int index) {
  assert(index == 0 || index == 1);
  if (index == 0) {
    return &m_promptView;
  } else {
    return &m_textField;
  }
}

void ConsoleEditCell::layoutSubviews(bool force) {
  KDSize promptSize = m_promptView.minimalSizeForOptimalDisplay();
  setChildFrame(&m_promptView,
                KDRect(KDPointZero, promptSize.width(), bounds().height()),
                force);
  setChildFrame(
      &m_textField,
      KDRect(KDPoint(promptSize.width(), KDCoordinate(0)),
             bounds().width() - promptSize.width(), bounds().height()),
      force);
}

void ConsoleEditCell::didBecomeFirstResponder() {
  Container::activeApp()->setFirstResponder(&m_textField);
  m_textField.setEditing(true);
}

void ConsoleEditCell::setEditing(bool isEditing) {
  m_textField.setEditing(isEditing);
}

void ConsoleEditCell::setText(const char *text) { m_textField.setText(text); }

void ConsoleEditCell::setPrompt(const char *prompt) {
  m_promptView.setText(prompt);
  layoutSubviews();
}

bool ConsoleEditCell::insertText(const char *text) {
  return m_textField.handleEventWithText(text);
}

void ConsoleEditCell::clearAndReduceSize() {
  setText("");
  // TODO
  /*
  size_t previousBufferSize = m_textField.draftTextBufferSize();
  assert(previousBufferSize > 1);
  m_textField.setDraftTextBufferSize(previousBufferSize - 1);*/
}

const char *ConsoleEditCell::shiftCurrentTextAndClear() {
  // TODO
  /* size_t previousBufferSize = m_textField.draftTextBufferSize();
   m_textField.setDraftTextBufferSize(previousBufferSize + 1);
   char *textFieldBuffer = const_cast<char *>(m_textField.text());
   char *newTextPosition = textFieldBuffer + 1;
   assert(previousBufferSize > 0);
   size_t copyLength = std::min(previousBufferSize - 1,
   strlen(textFieldBuffer)); memmove(newTextPosition, textFieldBuffer,
   copyLength); newTextPosition[copyLength] = 0; textFieldBuffer[0] = 0; return
   newTextPosition; */
  return const_cast<char *>(m_textField.text());
}

}  // namespace Code
