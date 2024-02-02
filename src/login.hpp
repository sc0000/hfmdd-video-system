#pragma once

#include <QWidget>
#include <QDockWidget>

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
  QString currentMailAddress;
  bool mailAddressIsValid;

  QString innerDir;

  bool verifyMailAddress();

private slots:
  void on_mailAddressLineEdit_textEdited(const QString& text);
  void on_newBookingButton_pressed();
  void on_manageBookingsButton_pressed();
  void on_toPTZControlsButton_pressed();
    
public:
  Login(QWidget *parent = nullptr);
  ~Login();
  static inline Login* getInstance() { return instance; }

};

