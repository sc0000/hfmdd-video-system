#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>

#include "backend.hpp"
#include "widgets.hpp"
// #include "fullscreen-dialog.hpp" // TODO: Remove
#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "settings-manager.hpp"
#include "mode-select.hpp"
#include "ui_login-dialog.h"
#include "login-dialog.hpp"

LoginDialog* LoginDialog::instance = NULL;

LoginDialog::LoginDialog(QWidget *parent)
  : FullScreenDialog(parent),
    ui(new Ui::LoginDialog),
    mailAddressIsValid(false)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");

  instance = this;
  ui->setupUi(this);
  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #FF0000 }");

  center(ui->masterWidget);

  setModal(false);
  hide();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::reload()
{
  raise();
  center(ui->masterWidget);
  
  ui->mailAddressLineEdit->clear();
  ui->passwordLineEdit->clear();
}

bool LoginDialog::verifyMailAddress()
{
  if (!mailAddressIsValid) {
    ui->reminderLabel->show();
    ui->reminderLabel->setText("Please enter a valid HfMDD email address! Only admin accounts require a passwords.");
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
  Backend::currentEmail = text;
  mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx

  for (const QString& suffix : mailSuffices)
  {
    if (Backend::currentEmail.endsWith(suffix))
    {
      mailAddressIsValid = true;
      SettingsManager::innerDirectory = Backend::currentEmail.chopped(suffix.length());
      ui->reminderLabel->setText(" ");
      break;
    }
  }

  if (Backend::currentEmail == Backend::adminEmail) 
    ui->passwordLineEdit->setDisabled(false);

  else 
    ui->passwordLineEdit->setDisabled(true);

  verifyMailAddress();
}

void LoginDialog::on_manageBookingsButton_pressed()
{
  if (!verifyMailAddress()) return;

  if (ui->passwordLineEdit->isEnabled() && ui->passwordLineEdit->text() != "pw") {
    Widgets::okDialog->display("Incorrect password");
    return;
  }

  fade(Widgets::modeSelect);
}