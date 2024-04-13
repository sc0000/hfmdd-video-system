#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>



QT_BEGIN_NAMESPACE
namespace Ui {
class ModeSelect;
}
QT_END_NAMESPACE

class ModeSelect : public QDialog
{
  Q_OBJECT
private:
  static ModeSelect* instance;
  Ui::ModeSelect* ui;

private:
  void repositionMasterWidget();

private slots:
  void on_quickModeButton_pressed();
  void on_bookModeButton_pressed();  
    
public:
  ModeSelect(QWidget *parent = nullptr);
  ~ModeSelect();

  static inline ModeSelect* getInstance() { return instance; }
  // inline QLineEdit* getMailAddressLineEdit() { return ui->mailAddressLineEdit; }
  void reload();
};

