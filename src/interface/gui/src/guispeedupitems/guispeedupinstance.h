/**
 * @file GuiSpeedupInstance.h
 * @author Yell

 * @brief
 * @version 0.1
 * @date 2021-11-29(V0.1)
 *
 *
 *
 */

#ifndef GUI_SPEEDUP_INSTANCE
#define GUI_SPEEDUP_INSTANCE
#include <vector>

#include "guispeedupitem.h"
#include "guispeedupwire.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GuiPinLayerShape {
 public:
  GuiPinLayerShape(int32_t z_order, QColor color, GuiSpeedupItemType type, GuiSpeedupItem* parent = nullptr) {
    _z_order   = z_order;
    _pin_shape = new GuiSpeedupWire(color, z_order, parent, type);
    _pin_shape->set_as_port();
  }

  GuiPinLayerShape(int32_t z_order, GuiSpeedupWire* pin_shape) {
    _z_order   = z_order;
    _pin_shape = pin_shape;
  }

  GuiPinLayerShape(const GuiPinLayerShape& other)            = default;
  GuiPinLayerShape(GuiPinLayerShape&& other)                 = default;
  GuiPinLayerShape& operator=(const GuiPinLayerShape& other) = default;
  GuiPinLayerShape& operator=(GuiPinLayerShape&& other)      = default;
  ~GuiPinLayerShape() {
    _z_order = 0;
    if (_pin_shape != nullptr) {
      delete _pin_shape;
      _pin_shape = nullptr;
    }
  };

  /// getter
  int32_t get_z_order() { return _z_order; }
  GuiSpeedupWire* get_pin_shape() { return _pin_shape; }

  /// setter
  void set_z_order(int32_t z_order) { _z_order = z_order; }
  void set_pin_shape(GuiSpeedupWire* pin_shape) { _pin_shape = pin_shape; }

 private:
  int32_t _z_order           = 0;
  GuiSpeedupWire* _pin_shape = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GuiSpeedupInstance : public GuiSpeedupItem {
 public:
  explicit GuiSpeedupInstance(GuiSpeedupItem* parent = nullptr) : GuiSpeedupItem(parent) {
    setPen(QColor(130, 130, 130));
    setBrush(QBrush(QColor(140, 140, 140), Qt::BrushStyle::Dense6Pattern));
    _brush = brush();
    _pen   = pen();
    setZValue(1);
  }

  virtual ~GuiSpeedupInstance() { clear(); }

  virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
  //   virtual QPainterPath shape() const override;
  //   virtual QRectF boundingRect() const override;

  virtual bool is_visible() override;
  virtual bool has_capacity() override {
    return get_rect_number() + get_pin_number() < GUI_ITEM_INSTANCE_MAX ? true : false;
  }

  /// getter
  //   virtual int32_t get_points_number() { }
  int32_t get_pin_number() { return _pin_list.size(); }
  std::vector<GuiPinLayerShape*>& get_pin_layer_shape_list() { return _pin_layer_shape_list; }

  /// setter
  void add_pin(qreal x_pos, qreal y_pos, qreal width) {
    if (width <= 0) {
      _pin_list.push_back(QRectF(x_pos - 0.02, y_pos - 0.02, 0.02, 0.02));
    } else {
      _pin_list.push_back(QRectF(x_pos - width / 2, y_pos - width / 2, width, width));
    }
  }

  /// only one pin shape item in one layer
  GuiSpeedupWire* add_pin_shape(QColor color, int32_t z_order) {
    GuiSpeedupWire* pin_shape_find = findPinShape(z_order);
    if (pin_shape_find != nullptr) {
      return pin_shape_find;
    }

    GuiPinLayerShape* pin_shape = new GuiPinLayerShape(z_order, color, get_type(), this);
    _pin_layer_shape_list.emplace_back(pin_shape);

    return pin_shape->get_pin_shape();
  }

  /// operator
  GuiSpeedupWire* findPinShape(int32_t z_order) {
    for (auto pin_shape : _pin_layer_shape_list) {
      if (pin_shape->get_z_order() == z_order) {
        return pin_shape->get_pin_shape();
      }
    }

    return nullptr;
  }

  void addPinShapeToScene(GuiGraphicsScene* scene) {
    for (auto& pin_shape : _pin_layer_shape_list) {
      if (pin_shape->get_pin_shape() != nullptr) {
        scene->addItem(pin_shape->get_pin_shape());
      }
    }
  }

  virtual GuiSpeedupItem* clone() override {
    GuiSpeedupInstance* new_item = new GuiSpeedupInstance(dynamic_cast<GuiSpeedupItem*>(this->parentItem()));
    new_item->set_bounding_rect(get_bounding_rect());
    new_item->setZValue(this->zValue());
    return dynamic_cast<GuiSpeedupItem*>(new_item);
  }

  virtual void clear() {
    _pin_list.clear();

    _pin_layer_shape_list.clear();

    GuiSpeedupItem::clear();
  }

  /// painter
  virtual void paintScaleTop(QPainter* painter, qreal lod);
  virtual void paintScale_1st(QPainter* painter, qreal lod);
  virtual void paintScale_2nd(QPainter* painter, qreal lod);
  virtual void paintScale_3rd(QPainter* painter, qreal lod);

 private:
  std::vector<QRectF> _pin_list;
  std::vector<GuiPinLayerShape*> _pin_layer_shape_list;
  QPen _pen;
  QBrush _brush;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GuiSpeedupInstanceList : public GuiSpeedupItemList {
 public:
  GuiSpeedupInstanceList(GuiGraphicsScene* scene) : GuiSpeedupItemList(scene, GuiSpeedupItemType::kInstance){};
  ~GuiSpeedupInstanceList() = default;

  /// getter
  /// find item by coordinate
  GuiSpeedupInstance* findItem(QPointF pt) { return dynamic_cast<GuiSpeedupInstance*>(get_item(pt)); }
  /// setter

  GuiSpeedupInstance* addCurrentItem(GuiSpeedupItem* parent = nullptr) {
    GuiSpeedupItem* new_item = new GuiSpeedupInstance(parent);
    new_item->setZValue(1);

    return dynamic_cast<GuiSpeedupInstance*>(add_current_item(new_item));
  }

  /// Operator
  virtual void init(QRectF boundingbox) override;
  virtual void addSceneItem(GuiSpeedupItem* item) {
    GuiSpeedupInstance* instance_item = dynamic_cast<GuiSpeedupInstance*>(item);
    instance_item->addPinShapeToScene(get_scene());
    GuiSpeedupItemList::addSceneItem(item);
  }

  virtual void finishCreateItem() override {
    for (GuiSpeedupItem* item : get_item_list()) {
      if (item != nullptr) {
        addSceneItem(item);
      }
    }
  }

  void clearData() {
#pragma omp parallel for
    for (GuiSpeedupItem* item : get_item_list()) {
      if (item != nullptr) {
        GuiSpeedupInstance* instance_item = dynamic_cast<GuiSpeedupInstance*>(item);
        instance_item->clear();
      }
    }
  }

 private:
};

#endif  // GUI_SPEEDUP_INSTANCE
