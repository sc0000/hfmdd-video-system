#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>

#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "path-manager.hpp"
#include "globals.hpp"
#include "mode-select.hpp"
#include "ui_login-dialog.h"
#include "login-dialog.hpp"

LoginDialog* LoginDialog::instance = NULL;

LoginDialog::LoginDialog(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::LoginDialog),
    mailAddressIsValid(false)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");

  instance = this;
  ui->setupUi(this);
  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #FF0000 }");

  repositionMasterWidget();

  showFullScreen();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::reload()
{
  repositionMasterWidget();
  ui->mailAddressLineEdit->clear();
  ui->passwordLineEdit->clear();
  show();
}

void LoginDialog::repositionMasterWidget()
{
  Globals::centerFullScreenWidget(ui->masterWidget);
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
  Globals::currentEmail = text;
  mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx

  for (const QString& suffix : mailSuffices)
  {
    if (Globals::currentEmail.endsWith(suffix))
    {
      mailAddressIsValid = true;
      PathManager::innerDirectory = Globals::currentEmail.chopped(suffix.length());
      ui->reminderLabel->setText(" ");
      break;
    }
  }

  if (Globals::currentEmail == Globals::adminEmail) {
    ui->passwordLineEdit->setDisabled(false);
    // PasswordDialog::instance(mailAddressIsValid, this);
  }

  else 
    ui->passwordLineEdit->setDisabled(true);

 

  verifyMailAddress();
}

void LoginDialog::on_manageBookingsButton_pressed()
{
  if (!verifyMailAddress()) return;

  if (ui->passwordLineEdit->isEnabled() && ui->passwordLineEdit->text() != "pw") {
    OkDialog::instance("Incorrect password", this);
    return;
  }

  ModeSelect* modeSelect = ModeSelect::getInstance();

  if (modeSelect) {
    modeSelect->show();
  }

  else {
    modeSelect = new ModeSelect(this);

    if (!modeSelect) return;

    modeSelect->exec();
  }

  hide();
}