#ifndef GUIGRAPHICSSCENE_H
#define GUIGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QString>

#include "guidatabase.h"
// #include "guidbinterface.h"
#include <QTimer>

#include "DrcViolationSpot.h"
#include "builder.h"
#include "file_cts.h"
#include "file_placement.h"
#include "guiitem.h"
#include "guitree.h"
#include "line.h"
#include "ruler.h"
#include "shape.h"

class DbSetup;

enum class GraphicMode { kNormal, kFast, kMax };

class GuiGraphicsScene : public QGraphicsScene {
  Q_OBJECT

 public:
  GuiGraphicsScene(QObject* parent = nullptr);
  virtual ~GuiGraphicsScene();

  /// getter
  QColor get_color(QString objectName);
  bool isEmpty() { return empty; }
  GraphicMode get_graphic_mode() { return _graphic_mode; }
  //   QGraphicsItem* get_selected_item() { return _selected_item; }
  //   QList<Ruler*> get_ruler_list() { return _ruler_list; }
  DbSetup* get_db_setup() { return _db_setup; }

  /// setter
  void set_graphic_mode(GraphicMode graphic_mode) { _graphic_mode = graphic_mode; }

  /// operator
  void createChip(std::map<std::string, std::list<std::string>> map);
  void createChip(std::vector<std::string> lef_paths, std::string def_path);
  void createChip(IdbBuilder* builder, std::string type = "");
  void createDrc(std::map<std::string, std::vector<idrc::DrcViolationSpot*>>& drc_db, int max_num = -1);
  void createClockTree(std::vector<iplf::CtsTreeNodeMap*>& node_list);
  void updateInstanceInFastMode(std::vector<iplf::FileInstance>& file_inst_list);

  void onDbChanged(DbSetup* db_setup);
  void fitView(qreal width, qreal height);
  void search(QString str);
  void viewRect(QRectF rect);
  void updateScene(std::string node_name, std::string parent_name);

  /// timer control
  void timerCreate() {
    _timer = new QTimer();
    connect(_timer, SIGNAL(timeout()), this, SLOT(timerOut()));
  }

  void timerKill() {
    if (_timer) {
      timerStop();
      _timer = nullptr;
    }
  }

  void timerStart(int ms = 1000) {
    _ms = ms;
    if (_timer) {
      _timer->start(_ms);
    }
  }

  void timerStop() {
    if (_timer) {
      _timer->stop();
    }
  }

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

 public slots:
  //   void setCurrentShape(Shape::Code s) {
  //     if (s != _shape_code) {
  //       _shape_code = s;
  //       qDebug() << _shape_code;
  //       bDraw = true;
  //     }
  //   }
  void timerOut();

 private:
  bool empty         = true;
  bool bDraw         = false;
  DbSetup* _db_setup = nullptr;
  int _update_mode   = 0;

  //   QGraphicsItem* _selected_item = nullptr;
  //   Shape* _shape                 = nullptr;

  //   QList<Ruler*> _ruler_list;
  GraphicMode _graphic_mode;
  //   Shape::Code _shape_code;

  /// timer
  QTimer* _timer = nullptr;
  int _ms        = 10;
};

#endif  // GUIGRAPHICSSCENE_H
