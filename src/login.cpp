#include <obs-module.h>
#include <obs-frontend-api.h>
#include "login.hpp"
#include "ui_login.h"
#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-box.hpp"

Login* Login::instance = NULL;

void login_load(void)
{
  QWidget* main_window =
    (QWidget*)obs_frontend_get_main_window();
    auto* tmp = new Login(main_window);
    obs_frontend_add_dock(tmp);
    tmp->setFloating(false);
}

Login::Login(QWidget *parent)
    : QDockWidget("Login", parent), ui(new Ui::Login)
{
  instance = this;
  ui->setupUi(this);
  setAllowedAreas(Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::NoDockWidgetFeatures);
  setTitleBarWidget(new QWidget());
  show();
}

Login::~Login()
{
    delete ui;
}

bool Login::verifyMailAddress()
{
  if (!mailAddressIsValid) {
     std::unique_ptr<MessageBox> messageBox = std::make_unique<MessageBox>("Please enter a valid email address!", this);
    messageBox->exec();
    return false;
  }

  return true;
}

void Login::on_mailAddressLineEdit_textEdited(const QString& text)
{
 currentMailAddress = text;

  mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx
  
  for (const QString& suffix : mailSuffices)
  {
    if (currentMailAddress.endsWith(suffix))
    {
      mailAddressIsValid = true;
      innerDir = currentMailAddress.chopped(suffix.length());
      break;
    }
  }

  QString styleSheet;

  if (mailAddressIsValid)
    styleSheet = "QLineEdit { border: 2px solid #48FF8B }";

  else
    styleSheet = "QLineEdit { border: 2px solid #FF5952 }";

  findChild<QLineEdit*>("mailAddressLineEdit")->setStyleSheet(styleSheet);
}

void Login::on_newBookingButton_pressed()
{
  if (!verifyMailAddress()) return;
}

void Login::on_manageBookingsButton_pressed()
{
  if (!verifyMailAddress()) return;
}

void Login::on_toPTZControlsButton_pressed()
{
  if (!verifyMailAddress()) return;

  hide();
  PTZControls::getInstance()->setFloating(false);
  PTZControls::getInstance()->show();
}