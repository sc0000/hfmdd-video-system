#pragma once

#include <QWidget>
#include <QDockWidget>
#include <QLineEdit>

#ifdef __cplusplus
extern "C" {
#endif

extern void login_load(void);

#ifdef __cplusplus
}
#endif

QT_BEGIN_NAMESPACE
namespace Ui {
class Login;
}
QT_END_NAMESPACE

class Login : public QDockWidget
{
  Q_OBJECT
private:
  static Login* instance;
  Ui::Login* ui;

private:
  QLineEdit* mailAddressLineEdit;
  bool mailAddressIsValid;

  bool verifyMailAddress();

private slots:
  void on_mailAddressLineEdit_textEdited(const QString& text);
  void on_manageBookingsButton_pressed();
    
public:
  Login(QWidget *parent = nullptr);
  ~Login();

  static inline Login* getInstance() { return instance; }
  inline QLineEdit* getMailAddressLineEdit() { return mailAddressLineEdit; }
};

