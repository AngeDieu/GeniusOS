#ifndef ESCHER_DROPDOWN_WIDGET_H
#define ESCHER_DROPDOWN_WIDGET_H

#include <escher/container.h>
#include <escher/dropdown_view.h>
#include <escher/menu_cell.h>

namespace Escher {

/* Widgets need a default constructor, which is not the case for dropdowns. This
 * widget allows the controller to own the dropdown, initialize it, and then
 * pass it to the widget. */
class DropdownWidget : public CellWidget {
 public:
  void setDropdown(Dropdown* dropdown) { m_dropdown = dropdown; }

  // CellWidget
  const View* view() const override { return m_dropdown; }
  Responder* responder() override { return m_dropdown; }
  void setHighlighted(bool highlighted) override {
    m_dropdown->setHighlighted(highlighted);
  }
  bool alwaysAlignWithLabelAsAccessory() const override { return true; }
  bool preventRightAlignedSubLabel(Type type) const override {
    return type == Type::Accessory;
  }

 private:
  Dropdown* m_dropdown;
};

}  // namespace Escher

#endif
