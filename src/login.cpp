#include <obs-module.h>
#include <obs-frontend-api.h>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "ui_login.h"
#include "login.hpp"

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
  : QDockWidget("Login", parent),
    ui(new Ui::Login),
    mailAddressIsValid(false)
{
  instance = this;
  ui->setupUi(this);
  setAllowedAreas(Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::NoDockWidgetFeatures);
  setTitleBarWidget(new QWidget());

  ui->reminderLabel->setMinimumHeight(ui->reminderLabel->fontMetrics().lineSpacing() * 2);

  show();
}

Login::~Login()
{
    delete ui;
}

bool Login::verifyMailAddress()
{
  if (!mailAddressIsValid) {
    ui->reminderLabel->setText("Please enter a valid HfMDD email address");
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #FF5952 }");
    return false;
  }

  else {
    ui->reminderLabel->setText(" ");
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #48FF8B }");
  }

  return true;
}

void Login::on_mailAddressLineEdit_textEdited(const QString& text)
{
  Globals::currentEmail = text;
  mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx

  for (const QString& suffix : mailSuffices)
  {
    if (Globals::currentEmail.endsWith(suffix))
    {
      mailAddressIsValid = true;
      PathManager::innerDirectory = Globals::currentEmail.chopped(suffix.length());
      break;
    }
  }

  if (Globals::currentEmail == Globals::oliversEmail)
    PasswordDialog::instance(mailAddressIsValid, this);

  verifyMailAddress();
}

void Login::on_manageBookingsButton_pressed()
{
  if (!verifyMailAddress()) return;

  BookingManager* bookingManager = BookingManager::getInstance();

  if (bookingManager) {
    bookingManager->loadBookings();
    bookingManager->show();
  }


  else {
    bookingManager = new BookingManager(this);

    if (!bookingManager) return;

    bookingManager->exec();
  }
}