#ifndef SHARED_MATH_APP_H
#define SHARED_MATH_APP_H

#include <apps/math_toolbox.h>
#include <apps/math_variable_box_controller.h>

#include "app_with_store_menu.h"

namespace Shared {

class MathApp : public AppWithStoreMenu {
 public:
  MathToolbox* defaultToolbox() override final { return &m_mathToolbox; }
  MathVariableBoxController* defaultVariableBox() override final {
    return &m_variableBoxController;
  }

 protected:
  using AppWithStoreMenu::AppWithStoreMenu;

 private:
  MathToolbox m_mathToolbox;
  MathVariableBoxController m_variableBoxController;
};

}  // namespace Shared
#endif