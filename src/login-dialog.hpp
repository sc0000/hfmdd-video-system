#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>
#include <QPaintEvent>

#include "fullscreen-dialog.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public FullScreenDialog, public Translatable
{
  Q_OBJECT
private:
  static LoginDialog* instance;
  Ui::LoginDialog* ui;

private:
  bool mailAddressIsValid;
  bool verifyMailAddress();

  QString reminderLabelText;
  QString passwordLineEditPlaceholderText;

  // virtual void paintEvent(QPaintEvent* event) override;

private slots:
  void on_mailAddressLineEdit_textChanged(const QString& text);
  void on_manageBookingsButton_pressed();
  void on_germanButton_pressed();
  void on_englishButton_pressed();
    
public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  static inline LoginDialog* getInstance() { return instance; }
  virtual void reload() override;

  virtual void translate(ELanguage language) override; 
};

