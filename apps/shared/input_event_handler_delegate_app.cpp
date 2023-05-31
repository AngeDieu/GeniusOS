#include "input_event_handler_delegate_app.h"

#include <apps/apps_container.h>
#include <escher/clipboard.h>
#include <string.h>

#include <cmath>

using namespace Escher;
using namespace Poincare;

namespace Shared {

InputEventHandlerDelegateApp::InputEventHandlerDelegateApp(
    Snapshot* snapshot, ViewController* rootViewController)
    : ::App(snapshot, rootViewController, I18n::Message::Warning),
      InputEventHandlerDelegate(),
      m_intrusiveStorageChangeFlag(false) {}

NestedMenuController* InputEventHandlerDelegateApp::toolbox() {
  return AppsContainer::sharedAppsContainer()->mathToolbox();
}

NestedMenuController* InputEventHandlerDelegateApp::variableBox() {
  return AppsContainer::sharedAppsContainer()->variableBoxController();
}

}  // namespace Shared
