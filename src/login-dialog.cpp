#include <obs-module.h>
#include <obs-frontend-api.h>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "ui_login-dialog.h"
#include "login-dialog.hpp"

LoginDialog* LoginDialog::instance = NULL;

// void LoginDialog_load(void)
// {
//   QWidget* main_window =
//     (QWidget*)obs_frontend_get_main_window();
//   auto* tmp = new LoginDialog(main_window);
//   obs_frontend_add_dock(tmp);
//   tmp->setFloating(true);
// }

LoginDialog::LoginDialog(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::LoginDialog),
    mailAddressIsValid(false)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");

  instance = this;
  ui->setupUi(this);
  mailAddressLineEdit = ui->mailAddressLineEdit;
  ui->masterWidget->move(geometry().center() - ui->masterWidget->geometry().center());
  // ui->manageBookingsButton->setStyleSheet("QPushButton { border: 1px solid #C8C8C8 }");
  showFullScreen();
  
  // setAllowedAreas(Qt::RightDockWidgetArea);
  // setFeatures(QDockWidget::NoDockWidgetFeatures);
  // setTitleBarWidget(new QWidget());

  // ui->reminderLabel->setMinimumHeight(ui->reminderLabel->fontMetrics().lineSpacing() * 2);

  // show();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::verifyMailAddress()
{
  if (!mailAddressIsValid) {
    ui->reminderLabel->setText("Please enter a valid HfMDD email address");
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #FF0000 }");
    return false;
  }

  else {
    ui->reminderLabel->setText(" ");
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #00FF00 }");
  }

  return true;
}

void LoginDialog::on_mailAddressLineEdit_textChanged(const QString& text)
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

void LoginDialog::on_manageBookingsButton_pressed()
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

  // hide();
}