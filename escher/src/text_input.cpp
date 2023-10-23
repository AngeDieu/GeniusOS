#include <assert.h>
#include <escher/text_input.h>
#include <ion/unicode/utf8_decoder.h>
#include <ion/unicode/utf8_helper.h>

#include <algorithm>

namespace Escher {
/* TextInput::ContentView */

void TextInput::ContentView::setFont(KDFont::Size font) {
  m_font = font;
  markWholeFrameAsDirty();
}

void TextInput::ContentView::setCursorLocation(const char* location) {
  assert(location != nullptr);
  assert(location >= editedText());
  const char* adjustedLocation =
      std::min(location, editedText() + editedTextLength());
  m_cursorLocation = adjustedLocation;
  layoutSubviews();
}

KDRect TextInput::ContentView::cursorRect() const {
  return glyphFrameAtPosition(editedText(), m_cursorLocation);
}

void TextInput::ContentView::updateSelection(
    const char* previousCursorLocation) {
  if (selectionIsEmpty()) {
    m_selectionStart = previousCursorLocation;
  }
  if (m_selectionStart == m_cursorLocation) {
    m_selectionStart = nullptr;
  }
  reloadRectFromAndToPositions(
      std::min(m_cursorLocation, previousCursorLocation),
      std::max(m_cursorLocation, previousCursorLocation));
}

bool TextInput::ContentView::resetSelection() {
  if (selectionIsEmpty()) {
    return false;
  }
  const char* previousStart = selectionLeft();
  const char* previousEnd = selectionRight();
  m_selectionStart = nullptr;
  reloadRectFromAndToPositions(previousStart, previousEnd);
  return true;
}

bool TextInput::ContentView::selectionIsEmpty() const {
  return m_selectionStart == nullptr;
}

void TextInput::ContentView::setAlignment(float horizontalAlignment,
                                          float verticalAlignment) {
  m_horizontalAlignment = horizontalAlignment;
  m_verticalAlignment = verticalAlignment;
  markWholeFrameAsDirty();
}

void TextInput::ContentView::reloadRectFromPosition(
    const char* position, bool includeFollowingLines) {
  markRectAsDirty(dirtyRectFromPosition(position, includeFollowingLines));
}

void TextInput::ContentView::reloadRectFromAndToPositions(const char* start,
                                                          const char* end) {
  if (start == end) {
    return;
  }
  KDRect startFrame = glyphFrameAtPosition(text(), start);
  KDRect endFrame = glyphFrameAtPosition(text(), end);
  bool onSameLine = startFrame.y() == endFrame.y();
  markRectAsDirty(KDRect(
      onSameLine ? startFrame.x() : 0, startFrame.y(),
      onSameLine ? endFrame.right() - startFrame.left() : bounds().width(),
      endFrame.bottom() - startFrame.top() + 1));
}

KDRect TextInput::ContentView::dirtyRectFromPosition(
    const char* position, bool includeFollowingLines) const {
  KDRect glyphRect = glyphFrameAtPosition(text(), position);
  if (!includeFollowingLines) {
    KDRect dirtyRect =
        KDRect(glyphRect.x(), glyphRect.y(), bounds().width() - glyphRect.x(),
               glyphRect.height());
    return dirtyRect;
  }
  KDRect dirtyRect = KDRect(0, glyphRect.y(), bounds().width(),
                            bounds().height() - glyphRect.y());
  return dirtyRect;
}

/* TextInput */

bool TextInput::removePreviousGlyph() {
  contentView()->removePreviousGlyph();
  scrollToCursor();
  return true;
}

bool TextInput::setCursorLocation(const char* location) {
  assert(location != nullptr);
  const char* adjustedLocation = std::max(location, text());
  willSetCursorLocation(&adjustedLocation);
  contentView()->setCursorLocation(adjustedLocation);
  scrollToCursor();
  return true;
}

void TextInput::scrollToCursor() {
  /* Technically, we do not need to overscroll in text input. However, we should
   * layout the scroll view before calling scrollToContentRect (in case the size
   * of the scroll view has changed) and then call scrollToContentRect which
   * calls another layout of the scroll view if the offset has evolved.
   *
   * In order to avoid requiring two layouts, we allow overscrolling in
   * scrollToContentRect, and the last layout of the scroll view corrects the
   * size of the scroll view only once. */
  if (bounds() == KDRectZero) {
    return;
  }
  KDRect cursorRect = contentView()->cursorRect();
  assert(cursorRect.top() >= 0 && cursorRect.right() >= 0 &&
         cursorRect.bottom() >= 0 && cursorRect.left() >= 0);
  scrollToContentRect(cursorRect);
}

void TextInput::deleteSelection() {
  ContentView* cv = contentView();
  assert(!cv->selectionIsEmpty());
  const float horizontalAlignment = cv->horizontalAlignment();
  if (horizontalAlignment == 0.0f) {
    cv->reloadRectFromPosition(cv->selectionLeft(), true);
  }
  bool cursorIsRightOfSelection = cv->selectionRight() == cv->cursorLocation();
  size_t removedLength = cv->deleteSelection();
  if (cursorIsRightOfSelection) {
    setCursorLocation(cv->cursorLocation() - removedLength);
  }
  layoutSubviews(
      true);  // Set the cursor frame by calling the subviews relayouting
  scrollToCursor();
}

void TextInput::setAlignment(float horizontalAlignment,
                             float verticalAlignment) {
  contentView()->setAlignment(horizontalAlignment, verticalAlignment);
}

bool TextInput::insertTextAtLocation(const char* text, char* location) {
  if (contentView()->insertTextAtLocation(text, location)) {
    /* We layout the scrollable view before scrolling to cursor because the
     * content size might have changed. */
    layoutSubviews();
    scrollToCursor();
    return true;
  }
  return false;
}

bool TextInput::removeEndOfLine() {
  if (privateRemoveEndOfLine()) {
    scrollToCursor();
    return true;
  }
  return false;
}

bool TextInput::moveCursorLeft(int step) {
  // Move the cursor to the left step times, or until limit is reached.
  int i = 0;
  bool canMove = true;
  while (canMove && i < step) {
    if (cursorLocation() <= text()) {
      assert(cursorLocation() == text());
      canMove = false;
    } else {
      UTF8Decoder decoder(text(), cursorLocation());
      canMove = setCursorLocation(decoder.previousGlyphPosition());
    }
    i++;
  }
  // true is returned if there was at least one successful cursor movement
  return (i > 1 || canMove);
}

bool TextInput::moveCursorRight(int step) {
  // Move the cursor to the right step times, or until limit is reached.
  int i = 0;
  bool canMove = true;
  while (canMove && i < step) {
    if (UTF8Helper::CodePointIs(cursorLocation(), UCodePointNull)) {
      canMove = false;
    } else {
      UTF8Decoder decoder(cursorLocation());
      canMove = setCursorLocation(decoder.nextGlyphPosition());
    }
    i++;
  }
  // true is returned if there was at least one successful cursor movement
  return (i > 1 || canMove);
}

bool TextInput::selectLeftRight(OMG::HorizontalDirection direction, bool all,
                                int step) {
  const char* previousCursorLoc = cursorLocation();
  if (!all) {
    bool moved =
        direction.isLeft() ? moveCursorLeft(step) : moveCursorRight(step);
    if (!moved) {
      return false;
    }
  } else {
    const char* t = text();
    const char* nextCursorLoc = direction.isLeft() ? t : t + strlen(t);
    willSetCursorLocation(&nextCursorLoc);
    if (previousCursorLoc == nextCursorLoc) {
      return false;
    }
    setCursorLocation(nextCursorLoc);
  }
  contentView()->updateSelection(previousCursorLoc);
  return true;
}

bool TextInput::privateRemoveEndOfLine() {
  return contentView()->removeEndOfLine();
}

}  // namespace Escher
