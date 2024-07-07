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
  bool verifyMailAddress();

  QString reminderLabelText;
  QString passwordLineEditPlaceholderText;

private slots:
  void on_mailAddressLineEdit_textChanged(const QString& text);
  void on_manageBookingsButton_clicked();
  void on_languageComboBox_currentTextChanged(const QString& text);
    
public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  static inline LoginDialog* getInstance() { return instance; }
  virtual void reload() override;

  virtual void translate(ELanguage language) override; 
};

