#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>

#include "language.hpp"
#include "fullscreen-dialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class ModeSelect;
}
QT_END_NAMESPACE

class ModeSelect : public FullScreenDialog, public Translatable
{
  Q_OBJECT
private:
  static ModeSelect* instance;
  Ui::ModeSelect* ui;

private slots:
  void on_quickModeButton_clicked();
  void on_bookModeButton_clicked();  
    
public:
  ModeSelect(QWidget *parent = nullptr);
  ~ModeSelect();

  static inline ModeSelect* getInstance() { return instance; }
  virtual void reload() override;

  virtual void translate(ELanguage language) override; 
};

