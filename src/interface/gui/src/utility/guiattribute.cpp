#include "guiattribute.h"

#include <iostream>

GuiAttribute* GuiAttribute::_instance = nullptr;

GuiAttribute::GuiAttribute() { initColorList(); }

void GuiAttribute::resetColor() { _default_color_list.clear(); }

/// hardcode
void GuiAttribute::initDefaultColorList() {
  //   _default_color_list.push_back(std::make_pair("POLY1", QColor(244, 164, 96)));
  //   _default_color_list.push_back(std::make_pair("CONT", QColor(0, 0, 255)));
  //   _default_color_list.push_back(std::make_pair("METAL1", QColor(0, 0, 255)));
  //   _default_color_list.push_back(std::make_pair("VIA12", QColor(255, 0, 0)));
  //   _default_color_list.push_back(std::make_pair("METAL2", QColor(255, 0, 0)));
  //   _default_color_list.push_back(std::make_pair("VIA23", QColor(0, 255, 0)));
  //   _default_color_list.push_back(std::make_pair("METAL3", QColor(0, 255, 0)));
  //   _default_color_list.push_back(std::make_pair("VIA34", QColor(208, 208, 0)));
  //   _default_color_list.push_back(std::make_pair("METAL4", QColor(208, 208, 0)));
  //   _default_color_list.push_back(std::make_pair("VIA45", QColor(165, 42, 42)));
  //   _default_color_list.push_back(std::make_pair("METAL5", QColor(165, 42, 42)));
  //   _default_color_list.push_back(std::make_pair("VIA56", QColor(255, 165, 0)));
  //   _default_color_list.push_back(std::make_pair("METAL6", QColor(255, 165, 0)));
  //   _default_color_list.push_back(std::make_pair("VIA67", QColor(208, 0, 208)));
  //   _default_color_list.push_back(std::make_pair("METAL7", QColor(208, 0, 208)));
  //   _default_color_list.push_back(std::make_pair("VIA78", QColor(0, 208, 208)));
  //   _default_color_list.push_back(std::make_pair("METAL8", QColor(0, 208, 208)));
  //   _default_color_list.push_back(std::make_pair("VIA89", QColor(0, 150, 150)));
  //   _default_color_list.push_back(std::make_pair("METAL9", QColor(0, 150, 150)));
  //   _default_color_list.push_back(std::make_pair("PA", QColor(139, 69, 19)));
  //   _default_color_list.push_back(std::make_pair("ALPA", QColor(139, 69, 19)));
}

void GuiAttribute::initColorList() {
  //   _color_list.push_back(QColor(244, 164, 96));
  //   _color_list.push_back(QColor(0, 0, 255));
  //   _color_list.push_back(QColor(0, 0, 255));
  //   _color_list.push_back(QColor(255, 0, 0));
  //   _color_list.push_back(QColor(255, 0, 0));
  //   _color_list.push_back(QColor(0, 255, 0));
  //   _color_list.push_back(QColor(0, 255, 0));
  //   _color_list.push_back(QColor(208, 208, 0));
  //   _color_list.push_back(QColor(208, 208, 0));
  //   _color_list.push_back(QColor(165, 42, 42));
  //   _color_list.push_back(QColor(165, 42, 42));
  //   _color_list.push_back(QColor(255, 165, 0));
  //   _color_list.push_back(QColor(255, 165, 0));
  //   _color_list.push_back(QColor(208, 0, 208));
  //   _color_list.push_back(QColor(208, 0, 208));
  //   _color_list.push_back(QColor(0, 208, 208));
  //   _color_list.push_back(QColor(0, 208, 208));
  //   _color_list.push_back(QColor(139, 69, 19));
  //   _color_list.push_back(QColor(139, 69, 19));
  _color_list.push_back(QColor(0, 0, 0));
  _color_list.push_back(QColor(100, 0, 0));
  _color_list.push_back(QColor(0, 100, 0));
  _color_list.push_back(QColor(0, 0, 100));
  _color_list.push_back(QColor(100, 100, 0));
  _color_list.push_back(QColor(100, 0, 100));
  _color_list.push_back(QColor(0, 100, 100));

  _color_list.push_back(QColor(150, 0, 0));
  _color_list.push_back(QColor(0, 150, 0));
  _color_list.push_back(QColor(0, 0, 150));
  _color_list.push_back(QColor(150, 150, 0));
  _color_list.push_back(QColor(150, 0, 150));
  _color_list.push_back(QColor(0, 150, 150));

  _color_list.push_back(QColor(200, 0, 0));
  _color_list.push_back(QColor(0, 200, 0));
  _color_list.push_back(QColor(0, 0, 200));
  _color_list.push_back(QColor(200, 200, 0));
  _color_list.push_back(QColor(200, 0, 200));
  _color_list.push_back(QColor(0, 200, 200));

  _color_list.push_back(QColor(255, 0, 0));
  _color_list.push_back(QColor(0, 255, 0));
  _color_list.push_back(QColor(0, 0, 255));
  _color_list.push_back(QColor(255, 255, 0));
  _color_list.push_back(QColor(255, 0, 255));
  _color_list.push_back(QColor(0, 255, 255));

  _color_list.push_back(QColor(100, 50, 50));
  _color_list.push_back(QColor(50, 100, 50));
  _color_list.push_back(QColor(50, 50, 100));

  _color_list.push_back(QColor(200, 100, 100));
  _color_list.push_back(QColor(100, 200, 100));
  _color_list.push_back(QColor(100, 100, 200));

  _color_list.push_back(QColor(200, 200, 50));
  _color_list.push_back(QColor(200, 50, 200));
  _color_list.push_back(QColor(50, 200, 200));

  _color_list.push_back(QColor(200, 50, 50));
  _color_list.push_back(QColor(50, 200, 50));
  _color_list.push_back(QColor(50, 50, 200));
}

void GuiAttribute::updateColorByLayers(std::vector<std::string> layer_list) {
  resetColor();
  for (auto layer : layer_list) {
    addLayer(layer);
  }
}

QColor GuiAttribute::getLayerColor(std::string layer) {
  std::transform(layer.begin(), layer.end(), layer.begin(), ::toupper);

  /// find default
  auto it = std::find_if(_default_color_list.begin(), _default_color_list.end(),
                         [layer](auto item) { return layer == item.first; });
  if (it != _default_color_list.end()) {
    return it->second;
  }

  return QColor(100, 100, 100);
}

QColor GuiAttribute::getLayerColor(int32_t z_oder) {
  return z_oder >= 0 && z_oder < _color_list.size() ? _color_list[z_oder] : QColor(100, 100, 100);
}

void GuiAttribute::addLayer(std::string layer) {
  std::transform(layer.begin(), layer.end(), layer.begin(), ::toupper);
  int size = _default_color_list.size();
  _default_color_list.push_back(std::make_pair(layer, getLayerColor(size)));
}
