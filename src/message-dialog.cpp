#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "booking.h"
#include "backend.hpp"
#include "widgets.hpp"
#include "ptz-controls.hpp"
#include "ui_ok-dialog.h"
#include "ui_ok-cancel-dialog.h"
#include "ui_password-dialog.h"
#include "ui_preset-dialog.h"

#include "message-dialog.hpp"


// TODO: Setup some form of inheritance structure here?

OkDialog::OkDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setModal(true);
  hide();
}

void OkDialog::display(const QString& message)
{
  ui->messageLabel->setText(message);
  show();
}

void OkDialog::on_okButton_pressed()
{
  hide();
}

OkCancelDialog::OkCancelDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkCancelDialog),
    decision(false)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setModal(true);
  hide();
}

void OkCancelDialog::display(const QString& message, bool& out)
{
  ui->messageLabel->setText(message);
  decision = out;
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

PasswordDialog::PasswordDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::PasswordDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  // setModal(true);
  hide();
}

void PasswordDialog::display()
{
  valid = false;
  exec();
}

void PasswordDialog::on_okButton_pressed()
{
  if (ui->passwordLineEdit->text() == Backend::adminPassword) {
    valid = true;
    hide();
  }

  else {
    Widgets::okDialog->display("Invalid password.");
    valid = false;
  }
}

void PasswordDialog::on_cancelButton_pressed()
{
  valid = false;
  hide();
}

PresetDialog::PresetDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::PresetDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setModal(true);
  hide();
}

void PresetDialog::display(Booking* booking)
{
  if (booking) {
    ui->presetNameLineEdit->setText(
      booking->date.toString() + ", " +
      booking->startTime.toString("HH:mm") + " - " +
      booking->event
    );
  }

  else {
    ui->presetNameLineEdit->setPlaceholderText("New Preset Name");
  }

  show();
}

void PresetDialog::on_okButton_pressed()
{
  PTZControls* ptzControls = PTZControls::getInstance();

  if (!ptzControls) return;

  ptzControls->setNewPresetName(ui->presetNameLineEdit->text());
  ptzControls->savePreset();
  hide();
}

void PresetDialog::on_cancelButton_pressed()
{
  hide();
}