#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "mode-select.hpp"
#include "quick-record.hpp"
#include "booking-manager.hpp"
#include "settings-manager.hpp"
#include "widgets.hpp"
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

  obs_frontend_add_dock(this);
  setFloating(true);
}

Login::~Login()
{
    delete ui;
}

void Login::on_startButton_pressed()
{
  Widgets::showFullScreenDialogs(true);

  Widgets::loginDialog->reload();
  hide();
}