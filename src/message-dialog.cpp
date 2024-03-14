#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "ui_ok-dialog.h"
#include "ui_ok-cancel-dialog.h"
#include "ui_password-dialog.h"
#include "ui_preset-dialog.h"
#include "booking.h"
#include "ptz-controls.hpp"
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

PresetDialog::PresetDialog(Booking* booking, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::PresetDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  presetNameLineEdit = findChild<QLineEdit*>("presetNameLineEdit");

  if (booking) {
    presetNameLineEdit->setText(
      booking->date.toString() + ", " +
      booking->startTime.toString("HH:mm") + " - " +
      booking->event
    );
  }

  else {
    presetNameLineEdit->setPlaceholderText("New Preset Name");
  }
}

void PresetDialog::instance(Booking* booking, QWidget* parent)
{
  std::unique_ptr<PresetDialog> messageBox = std::make_unique<PresetDialog>(booking, parent);
  messageBox->exec();
}


void PresetDialog::on_okButton_pressed()
{
  PTZControls* ptzControls = PTZControls::getInstance();

  if (!ptzControls || !presetNameLineEdit) return;

  ptzControls->setNewPresetName(presetNameLineEdit->text());
  ptzControls->savePreset();
  hide();
}

void PresetDialog::on_cancelButton_pressed()
{
  hide();
}