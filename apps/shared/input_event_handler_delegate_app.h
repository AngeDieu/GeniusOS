#ifndef SHARED_INPUT_EVENT_HANDLER_DELEGATE_APP_H
#define SHARED_INPUT_EVENT_HANDLER_DELEGATE_APP_H

#include <escher/app.h>
#include <escher/input_event_handler_delegate.h>
#include <escher/nested_menu_controller.h>

#include "store_menu_controller.h"

/* TODO: This class should be refactored/deleted as well as
 * TextFieldDelegateApp and LayoutFieldDelegateApp. */

namespace Shared {

class InputEventHandlerDelegateApp : public Escher::App,
                                     public Escher::InputEventHandlerDelegate {
 public:
  virtual ~InputEventHandlerDelegateApp() = default;
  Escher::NestedMenuController* toolbox() override;
  Escher::NestedMenuController* variableBox() override;

  virtual void prepareForIntrusiveStorageChange() {
    assert(!m_intrusiveStorageChangeFlag);
    m_intrusiveStorageChangeFlag = true;
  }
  virtual void concludeIntrusiveStorageChange() {
    assert(m_intrusiveStorageChangeFlag);
    m_intrusiveStorageChangeFlag = false;
  }

 protected:
  InputEventHandlerDelegateApp(Snapshot* snapshot,
                               Escher::ViewController* rootViewController);
  bool m_intrusiveStorageChangeFlag;
};

}  // namespace Shared

#endif
