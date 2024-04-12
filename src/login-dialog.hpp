#pragma once

#include <QWidget>
#include <QDialog>
#include <QLineEdit>

#ifdef __cplusplus
extern "C" {
#endif

extern void LoginDialog_load(void);

#ifdef __cplusplus
}
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class LoginDialog;
}
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
  Q_OBJECT
private:
  static LoginDialog* instance;
  Ui::LoginDialog* ui;

private:
  bool mailAddressIsValid;

  void repositionMasterWidget();
  bool verifyMailAddress();

private slots:
  void on_mailAddressLineEdit_textChanged(const QString& text);
  void on_manageBookingsButton_pressed();
    
public:
  LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();

  static inline LoginDialog* getInstance() { return instance; }
  // inline QLineEdit* getMailAddressLineEdit() { return ui->mailAddressLineEdit; }
  void reload();
};

