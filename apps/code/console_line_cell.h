#ifndef CODE_CONSOLE_LINE_CELL_H
#define CODE_CONSOLE_LINE_CELL_H

#include <escher/highlight_cell.h>
#include <escher/message_text_view.h>
#include <escher/responder.h>
#include <escher/scrollable_view.h>
#include <escher/scroll_view_data_source.h>
#include <assert.h>

#include "console_line.h"

namespace Code {

class ConsoleLineCell : public HighlightCell, public Responder {
public:
  ConsoleLineCell(Responder * parentResponder = nullptr);
  void setLine(ConsoleLine line);

  /* HighlightCell */
  void setHighlighted(bool highlight) override;
  void reloadCell() override;

  /* View */
  int numberOfSubviews() const override;
  View * subviewAtIndex(int index) override;
  void layoutSubviews() override;

  /* Responder */
  void didBecomeFirstResponder() override;
private:
  class ScrollableConsoleLineView : public ScrollableView, public ScrollViewDataSource {
  public:
    class ConsoleLineView : public HighlightCell {
    public:
      ConsoleLineView();
      void setLine(ConsoleLine * line);
      void drawRect(KDContext * ctx, KDRect rect) const override;
      KDSize minimalSizeForOptimalDisplay() const override;
    private:
      ConsoleLine * m_line;
    };

    ScrollableConsoleLineView(Responder * parentResponder);
    KDSize minimalSizeForOptimalDisplay() const override;
    ConsoleLineView * consoleLineView() { return &m_consoleLineView; }
  private:
    ConsoleLineView m_consoleLineView;
  };
  MessageTextView m_promptView;
  ScrollableConsoleLineView m_scrollableView;
  ConsoleLine m_line;
};

}

#endif
