#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>

#include "fullscreen-dialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public FullScreenDialog
{
  Q_OBJECT
private:
  static LoginDialog* instance;
  Ui::LoginDialog* ui;

private:
  bool mailAddressIsValid;

  bool verifyMailAddress();

private slots:
  void on_mailAddressLineEdit_textChanged(const QString& text);
  void on_manageBookingsButton_pressed();
    
public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  static inline LoginDialog* getInstance() { return instance; }
  virtual void reload() override;
};

