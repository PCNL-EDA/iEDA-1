#ifndef FILEIMPORT_H
#define FILEIMPORT_H

#include <QtWidgets>
#include <list>
#include <map>
#include <string>

class FileImport : public QDialog {
  Q_OBJECT
 public:
  explicit FileImport(QWidget* parent = nullptr);
  ~FileImport();

  /// getter
  std::map<std::string, std::list<std::string>> getFilePath() { return _fileMap; }
  std::string get_tech_file() { return _tech_file; }

  /// operator
  int showDialog() {
    show();
    return exec();
  }

 private:
  /// init
  void init();
  void initDirLayout();
  void initTechLayout();
  void initListLayout();
  void initListItem();
  void clearListItem();
  void initControl();
  /// operation
  void openFile();
  void openTechFile();
  void okAct();
  void allSelect();
  void cancelAct();
  void setItemChecked(QListWidgetItem* item);

  QVBoxLayout* _topLayout = nullptr;
  /// directory
  QLineEdit* _edit_directory     = nullptr;
  QHBoxLayout* _layout_directory = nullptr;
  QPushButton* _btn_open         = nullptr;
  /// tech directory
  QLineEdit* _tech_directory = nullptr;
  QHBoxLayout* _layout_tech  = nullptr;
  QPushButton* _btn_tech     = nullptr;
  /// select
  QHBoxLayout* _select_layout  = nullptr;
  QPushButton* _btn_select_all = nullptr;
  /// list
  QVBoxLayout* _layout_list = nullptr;
  QListWidget* _file_list   = nullptr;
  QHBoxLayout* _control     = nullptr;

  /// DEF&LEF Title
  QListWidgetItem* _list_def = nullptr;
  QListWidgetItem* _list_lef = nullptr;

  /// btn
  QPushButton* _ok = nullptr;
  QPushButton* _cancel;

  /// files
  QString _directory;
  std::map<std::string, std::list<std::string>> _fileMap;
  std::string _tech_file;

  int _def_index;
  int _lef_index;
 signals:
};
class ItemWidget : public QWidget {
  Q_OBJECT
 public:
  explicit ItemWidget(const QFileInfo& info, QWidget* parent = nullptr);
  void check();
  QFileInfo getFileInfo();
  bool isChecked();

 private:
  QCheckBox* checkBox = nullptr;
  QFileInfo fileInfo;
 signals:
};

#endif  // FILEIMPORT_H
