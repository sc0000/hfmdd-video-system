#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "mail-handler.hpp"
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
  : AnimatedDialog(parent),
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

  ui->messageLabel->setStyleSheet("QLabel { border: 1px solid rgb( 254, 253, 254); }");

  setModal(true);
  hide();
}

void OkDialog::display(const QString& message, bool triggerLogout)
{
  m_triggerLogout = triggerLogout;
  ui->messageLabel->setText(message);

  ui->okButton->setAttribute(Qt::WA_UnderMouse, false);

  fade();
}

void OkDialog::on_okButton_clicked()
{
  if (m_triggerLogout) {
    m_triggerLogout = false;
    Widgets::ptzControls->logout();
  }
  
  fade();
}

OkCancelDialog::OkCancelDialog(QWidget* parent)
  : AnimatedDialog(parent),
    ui(new Ui::OkCancelDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; border: 1px rgb(31, 30, 31); }");

  ui->okButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->messageLabel->setStyleSheet("QLabel { border: 1px solid rgb( 254, 253, 254); }");

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

  ui->okButton->setAttribute(Qt::WA_UnderMouse, false);
  ui->cancelButton->setAttribute(Qt::WA_UnderMouse, false);

  fade();
  return exec();
}

void okCancelDialogAccept()
{
  if (Widgets::okCancelDialog)
    Widgets::okCancelDialog->accept();
}

void okCancelDialogReject()
{
  if (Widgets::okCancelDialog)
    Widgets::okCancelDialog->reject();
}

void OkCancelDialog::on_okButton_clicked()
{
  fade(&okCancelDialogAccept);
}

void OkCancelDialog::on_cancelButton_clicked()
{
  fade(&okCancelDialogReject);
}

void passwordDialogAccept()
{
  if (Widgets::passwordDialog)
    Widgets::passwordDialog->accept();
}

void passwordDialogReject()
{
  if (Widgets::passwordDialog)
    Widgets::passwordDialog->reject();
}

PasswordDialog::PasswordDialog(QWidget* parent)
  : AnimatedDialog(parent),
    ui(new Ui::PasswordDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; border: 1px rgb(31, 30, 31); }");

  ui->okButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->passwordWidget->setStyleSheet("QWidget { border: 1px solid rgb( 254, 253, 254); }");
  ui->passwordLabel->setStyleSheet("QLabel { border: none; }");

  ui->cancelButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->passwordLineEdit->setStyleSheet(
    "QLineEdit { background-color: rgb(42,130,218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
  );

  setModal(true);
  hide();
}

int PasswordDialog::display()
{
  ui->passwordLineEdit->clear();

  ui->okButton->setAttribute(Qt::WA_UnderMouse, false);
  ui->cancelButton->setAttribute(Qt::WA_UnderMouse, false);

  fade();
  return exec();
}

void PasswordDialog::on_okButton_clicked()
{
  if (ui->passwordLineEdit->text() == MailHandler::adminPassword) 
    fade(&passwordDialogAccept);
  
  else 
    Widgets::okDialog->display("Invalid password.");
}

void PasswordDialog::on_cancelButton_clicked()
{
  fade(&passwordDialogReject);
}

PresetDialog::PresetDialog(QWidget* parent)
  : AnimatedDialog(parent),
    ui(new Ui::PresetDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; border: 1px rgb(31, 30, 31); }");

  ui->okButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->presetNameWidget->setStyleSheet("QWidget { border: 1px solid rgb( 254, 253, 254); }");
  ui->presetNameLabel->setStyleSheet("QLabel { border: none; }");

  ui->cancelButton->setStyleSheet(
    "QPushButton { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(31, 30, 31); }"
    "QPushButton:pressed { background-color: rgb(254, 253, 254); color: rgb(31, 30, 31); border: 1px solid rgb(31, 30, 31); }"
  );

  ui->presetNameLineEdit->setStyleSheet(
    "QLineEdit { background-color: rgb(42,130,218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
  );

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

  fade();
}

void PresetDialog::on_okButton_clicked()
{
  PTZControls* ptzControls = Widgets::ptzControls;

  if (!ptzControls) return;

  ptzControls->setNewPresetName(ui->presetNameLineEdit->text());
  ptzControls->savePreset();
  fade();
}

void PresetDialog::on_cancelButton_clicked()
{
  fade();
}

InfoDialog::InfoDialog(QWidget* parent)
 :  AnimatedDialog(parent),
    ui(new Ui::InfoDialog),
    button(nullptr),
    handlebar(nullptr)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet("QWidget { background-color: rgb(42,130,218); color: rgb(254, 253, 254); font: 12pt 'DaxOT'; }");
  
  ui->masterWidget->setFixedWidth(width() - 4);
  ui->masterWidget->setFixedHeight(height() - 32);
  ui->masterWidget->move(QPoint(4, 32));

  ui->messageLabel->setStyleSheet("QLabel { border: 1px solid rgb( 254, 253, 254); }");

  handlebar = new Handlebar(this, EHandlebarStyle::Blue);

  setModal(true);
  hide();
}

void InfoDialog::display(const QString& message, QPushButton* activatingButton, const int offsetX, const int offsetY)
{
  if (activatingButton) 
    button = activatingButton;

  ui->messageLabel->setText(message);

  QPushButton* closeButton = handlebar->getCloseButton();

  if (!closeButton) return;
  
  closeButton->setAttribute(Qt::WA_UnderMouse, false);

  fade();
}

void InfoDialog::fade(void (*result)(void))
{
  if (button) {
    button->setStyleSheet(
      "QPushButton { color: rgb(254, 253, 254); background-color: rgb(31, 30, 31); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(42,130,218); }"
    );

    button = nullptr;
  }

  AnimatedDialog::fade(result);
}