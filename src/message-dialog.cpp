#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "ui_ok-dialog.h"
#include "ui_ok-cancel-dialog.h"
#include "ui_password-dialog.h"
#include "message-dialog.hpp"

// TODO: Setup some form of inheritance structure here?

OkDialog::OkDialog(const QString& message, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkDialog)
{
  ui->setupUi(this);
  
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  findChild<QLabel*>("messageLabel")->setText(message);
}

void OkDialog::instance(const QString& message, QWidget* parent)
{
  std::unique_ptr<OkDialog> messageBox = std::make_unique<OkDialog>(message, parent);
  messageBox->exec();
}

void OkDialog::on_okButton_pressed()
{
  hide();
}

OkCancelDialog::OkCancelDialog(const QString& message, bool& out, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkCancelDialog), 
    decision(out)
{
  ui->setupUi(this);
  
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  findChild<QLabel*>("messageLabel")->setText(message);
}

void OkCancelDialog::instance(const QString& message, bool& out, QWidget* parent)
{
  std::unique_ptr<OkCancelDialog> messageBox = std::make_unique<OkCancelDialog>(message, out, parent);
  messageBox->exec();
}

void OkCancelDialog::on_okButton_pressed()
{
  decision = true;
  hide();
}

void OkCancelDialog::on_cancelButton_pressed()
{
  decision = false;
  hide();
}

PasswordDialog::PasswordDialog(bool& out, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::PasswordDialog),
    valid(out)
{
  ui->setupUi(this);

  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
}

void PasswordDialog::instance(bool& out, QWidget* parent)
{
  std::unique_ptr<PasswordDialog> messageBox = std::make_unique<PasswordDialog>(out, parent);
  messageBox->exec();
}

void PasswordDialog::on_okButton_pressed()
{
  if (findChild<QLineEdit*>("passwordLineEdit")->text() == "pw") {
    valid = true;
    hide();
  }

  else {
    OkDialog::instance("Invalid password.", this);
    valid = false;
  }
}

void PasswordDialog::on_cancelButton_pressed()
{
  valid = false;
  hide();
}  