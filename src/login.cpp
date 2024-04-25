#include <obs-module.h>
#include <obs-frontend-api.h>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "login-dialog.hpp"
#include "ui_login.h"
#include "login.hpp"

Login* Login::instance = NULL;

void login_load(void)
{
  QWidget* main_window =
    (QWidget*)obs_frontend_get_main_window();
  auto* tmp = new Login(main_window);
  obs_frontend_add_dock(tmp);
  tmp->setFloating(true);
}

Login::Login(QWidget *parent)
  : QDockWidget("Login", parent),
    ui(new Ui::Login)
{
  instance = this;
  ui->setupUi(this);

  show();
}

Login::~Login()
{
    delete ui;
}

void Login::on_startButton_pressed()
{
  LoginDialog* loginDialog = new LoginDialog(this);
  loginDialog->exec();
}