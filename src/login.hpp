#pragma once

#include <QWidget>
#include <QDockWidget>
#include <QLineEdit>

#include "language.hpp"

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

class Login : public QDockWidget, public Translatable
{
  Q_OBJECT
private:
  static Login* instance;
  Ui::Login* ui;

private slots:
  void on_startButton_pressed();
    
public:
  Login(QWidget *parent = nullptr);
  ~Login();

  static inline Login* getInstance() { return instance; }
  
  virtual void translate(ELanguage language) override; 
};

