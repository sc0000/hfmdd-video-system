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
#include "ui_info-dialog.h"

#include "message-dialog.hpp"


// TODO: Setup some form of inheritance structure here?

OkDialog::OkDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; border: 1px rgb(31, 30, 31); }");

  ui->okButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  setModal(true);
  hide();
}

void OkDialog::display(const QString& message, bool triggerLogout)
{
  m_triggerLogout = triggerLogout;
  ui->messageLabel->setText(message);
  show();
  raise();
}

void OkDialog::on_okButton_clicked()
{
  if (m_triggerLogout) {
    m_triggerLogout = false;
    Widgets::ptzControls->logout();
  }
  
  hide();
}

OkCancelDialog::OkCancelDialog(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::OkCancelDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; }");

  ui->okButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->cancelButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  setModal(true);
  hide();
}

int OkCancelDialog::display(const QString& message, const bool colorBlack)
{
  ui->messageLabel->setText(message);
  raise();
  return exec();
}

void OkCancelDialog::on_okButton_clicked()
{
  accept();
  hide();
}

void OkCancelDialog::on_cancelButton_clicked()
{
  reject();
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

void PasswordDialog::on_okButton_clicked()
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

void PasswordDialog::on_cancelButton_clicked()
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

void PresetDialog::on_okButton_clicked()
{
  PTZControls* ptzControls = PTZControls::getInstance();

  if (!ptzControls) return;

  ptzControls->setNewPresetName(ui->presetNameLineEdit->text());
  ptzControls->savePreset();
  hide();
}

void PresetDialog::on_cancelButton_clicked()
{
  hide();
}

InfoDialog::InfoDialog(QWidget* parent)
 : QDialog(parent),
    ui(new Ui::InfoDialog),
    button(nullptr)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; }");
  
  ui->messageLabel->setStyleSheet("QLabel { border: 1px solid rgb( 254, 253, 254); }");

  ui->closeButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); }"
  );

  setModal(false);
  hide();
}

void InfoDialog::display(const QString& message, QPushButton* activatingButton, const int offsetX, const int offsetY)
{
  if (activatingButton) 
    button = activatingButton;

  ui->messageLabel->setText(message);
  show();
  raise();
}

void InfoDialog::on_closeButton_clicked()
{
  if (button) {
    button->setStyleSheet(
      "QPushButton { color: rgb(254, 253, 254); background-color: rgb(31, 30, 31); border: 1px solid rgb(254, 253, 254); }"
      // "QPushButton:hover { background-color: rgb(42,130,218); }"
    );

    button = nullptr;
  }

  hide();
}