#include "DrcLayer.h"

namespace idrc {

// SpacingRangeRule* DrcRoutingLayer::add_spacing_range_rule()
// {
//   SpacingRangeRule* spacing_range_rule = new SpacingRangeRule();
//   _spacing_range_rule_list.push_back(spacing_range_rule);
//   return spacing_range_rule;
// }

int DrcRoutingLayer::getRoutingSpacing(int width)
{
  for (auto& spacingRangeRule : _spacing_range_rule_list) {
    if (width >= spacingRangeRule->get_min_width() && width <= spacingRangeRule->get_max_width()) {
      return spacingRangeRule->get_spacing();
    }
  }
  return _min_spacing;
}

int DrcRoutingLayer::getLayerMaxRequireSpacing(DrcRect* target_rect)
{
  int spacing = _min_spacing;
  if (isSpacingTable()) {
    spacing = _spacing_table->get_parallel_spacing(target_rect->getWidth(), target_rect->getLength());
  } else {
    for (auto& spacing_range_rule : _spacing_range_rule_list) {
      int range_rule_spacing = spacing_range_rule->get_spacing();
      if (spacing < range_rule_spacing) {
        spacing = range_rule_spacing;
      }
    }
  }

  return spacing;
}

void DrcRoutingLayer::clear_density_rule()
{
  if (_density_rule != nullptr) {
    delete _density_rule;
    _density_rule = nullptr;
  }
}
void DrcRoutingLayer::clear_spacing_range_rule_list()
{
  for (SpacingRangeRule* spacing_range_rule : _spacing_range_rule_list) {
    if (spacing_range_rule != nullptr) {
      delete spacing_range_rule;
      spacing_range_rule = nullptr;
    }
  }
}

}  // namespace idrc