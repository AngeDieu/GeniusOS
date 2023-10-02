#include <escher/selectable_table_view.h>
#include <escher/selectable_table_view_delegate.h>

namespace Escher {

KDPoint SelectableTableViewDelegate::actualOffset(
    const SelectableTableView* t) const {
  return t->contentOffset();
}

}  // namespace Escher