/**
 * @file guicontroltreeitem.h
 * @author Wang Jun (wen8365@gmail.com)
 * @brief Custom tree item in control tree
 * @version 0.1
 * @date 2021-07-02
 * 
 *
 * 
 */

#ifndef GUITCONTROLTREEITEM_H
#define GUITCONTROLTREEITEM_H

#include <QTreeWidgetItem>

#include "guitreeitemcolumn.h"

class GuiControlTreeItem : public QTreeWidgetItem {
 public:
  explicit GuiControlTreeItem(const QString &label) : QTreeWidgetItem() {
    if (!_is_column_cfg_loaded) {
      _is_column_cfg_loaded = true;
      _item_column.loadConfig();
    }
    setText(_item_column.labelCol(), label);
  };
  ~GuiControlTreeItem() = default;
  /**
   * @brief Get the column number of label.
   * @return
   */
  inline static int labelCol() { return _item_column.labelCol(); }
  /**
   * @brief Get the column name with the given column number.
   * Column name does not display on view but marks what job the Column does.
   * @return
   */
  inline static QString getColumnName(int column) {
    return _item_column.getColumn(column).name;
  }
  void lockColumns(QList<int> columns);
  void unlockColumns(QList<int> columns);
  bool isLocked(int column) { return _lock_columns.value(column); };
  void addSlave(int master, int slave);
  bool hasSlaves(int column) { return _m_s.find(column) != _m_s.end(); };
  QList<int> getSlaves(int master) { return _m_s.value(master); };
  void initColumns(Qt::CheckState state = Qt::Checked);
  QString get_label() { return text(_item_column.labelCol()).trimmed(); };
  void set_label(const QString &label) {
    setText(_item_column.labelCol(), label);
  };

 private:
  static GuiTreeItemColumn _item_column;
  static inline bool _is_column_cfg_loaded = false;
  QMap<int, bool> _lock_columns;
  QMap<int, QList<int>> _m_s;  // master and it's slaves
};

extern GuiTreeItemColumn _item_column;
#endif  // GUITCONTROLTREEITEM_H
