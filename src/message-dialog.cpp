/* Different kinds of message dialogs
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "mail-handler.hpp"
#include "widgets.hpp"
#include "ptz-controls.hpp"
#include "styles.hpp"
#include "ui_ok-dialog.h"
#include "ui_ok-cancel-dialog.h"
#include "ui_password-dialog.h"
#include "ui_preset-dialog.h"
#include "ui_info-dialog.h"
#include "ui_admin-mail-dialog.h"

#include "message-dialog.hpp"

OkDialog::OkDialog(QWidget* parent)
  : AnimatedDialog(parent),
    ui(new Ui::OkDialog)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet(Styles::messageBox);

  ui->okButton->setStyleSheet(Styles::blueButton);
  ui->messageLabel->setStyleSheet(Styles::framedLabel);

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

  setStyleSheet(Styles::messageBox);

  ui->okButton->setStyleSheet(Styles::blueButton);
  ui->cancelButton->setStyleSheet(Styles::blueButton);
  ui->messageLabel->setStyleSheet(Styles::framedLabel);

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

  setStyleSheet(Styles::messageBox);

  ui->okButton->setStyleSheet(Styles::blueButton);
  ui->cancelButton->setStyleSheet(Styles::blueButton);
  ui->passwordWidget->setStyleSheet(Styles::framedWidget);
  ui->passwordLabel->setStyleSheet(Styles::unframedLabel);
  ui->passwordLineEdit->setStyleSheet(Styles::blueLineEdit);

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

void PasswordDialog::updateTexts()
{
  ui->okButton->setText(TextHandler::getText(ID::INPUT_CONFIRM));
  ui->cancelButton->setText(TextHandler::getText(ID::INPUT_CANCEL));
  ui->passwordLabel->setText(TextHandler::getText(ID::INPUT_PASSWORD));
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

  setStyleSheet(Styles::messageBox);

  ui->okButton->setStyleSheet(Styles::blueButton);
  ui->cancelButton->setStyleSheet(Styles::blueButton);
  ui->presetNameWidget->setStyleSheet(Styles::framedWidget);
  ui->presetNameLabel->setStyleSheet(Styles::unframedLabel);
  ui->presetNameLineEdit->setStyleSheet(Styles::blueLineEdit);

  setModal(true);
  hide();
}

void PresetDialog::display(Booking* booking)
{
  ui->presetNameLineEdit->setText("");

  fade();
}

void PresetDialog::updateTexts()
{
  ui->okButton->setText(TextHandler::getText(ID::INPUT_CONFIRM));
  ui->cancelButton->setText(TextHandler::getText(ID::INPUT_CANCEL));
  ui->presetNameLabel->setText(TextHandler::getText(ID::INPUT_PRESET));
}

void PresetDialog::on_okButton_clicked()
{
  const QString& name = ui->presetNameLineEdit->text();

  bool justWhiteSpace = true;

  for (const QChar c : name) {
    if (c.isLetterOrNumber())
      justWhiteSpace = false;
  }

  if (name == "" || justWhiteSpace) {
    Widgets::okDialog->display(
      TextHandler::getText(ID::CONTROLS_PRESET_NO_NAME)
    );

    ui->presetNameLineEdit->setText("");
    return;
  }

  fade();

  PTZControls* ptzControls = Widgets::ptzControls;

  if (!ptzControls) return;

  ptzControls->setNewPresetName(ui->presetNameLineEdit->text());
  ptzControls->savePreset();
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

  setStyleSheet(Styles::unframedMessageBox);
  
  ui->masterWidget->setFixedWidth(width() - 4);
  ui->masterWidget->setFixedHeight(height() - 32);
  ui->masterWidget->move(QPoint(4, 32));

  ui->messageLabel->setStyleSheet(Styles::framedLabel);

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
  AnimatedDialog::fade(result);

  if (isVisible())
    button->setStyleSheet(Styles::infoButtonOn);

  else 
    button->setStyleSheet(Styles::infoButtonOff);
}

AdminMailDialog::AdminMailDialog(QWidget* parent)
 :  AnimatedDialog(parent),
    ui(new Ui::AdminMailDialog),
    handlebar(nullptr),
    m_booking(nullptr),
    locale(QLocale::German)
{
  ui->setupUi(this);
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);

  setStyleSheet(Styles::unframedMessageBox);
  
  ui->masterWidget->setFixedWidth(width() - 4);
  ui->masterWidget->setFixedHeight(height() - 32);
  ui->masterWidget->move(QPoint(4, 32));

  // ui->messageLabel->setStyleSheet("QLabel { border: 1px solid rgb( 254, 253, 254); }");

  handlebar = new Handlebar(this, EHandlebarStyle::Blue, "Mail Editor");

  ui->okButton->setStyleSheet(Styles::blueButton);
  ui->cancelButton->setStyleSheet(Styles::blueButton);
  ui->languageButton->setStyleSheet(Styles::blueButton);
  ui->mailSubjectLineEdit->setStyleSheet(Styles::blueLineEdit);
  ui->mailBodyTextEdit->setStyleSheet(Styles::blueTextEdit);

  setModal(true);
  hide();
}

void AdminMailDialog::display(EAdminMailType type, const Booking* booking, const Booking* originalEditedBooking)
{
  ui->okButton->setAttribute(Qt::WA_UnderMouse, false);
  ui->cancelButton->setAttribute(Qt::WA_UnderMouse, false);
  ui->languageButton->setAttribute(Qt::WA_UnderMouse, false);

  QPushButton* closeButton = handlebar->getCloseButton();
  
  if (closeButton)
    closeButton->setAttribute(Qt::WA_UnderMouse, false);

  m_type = type;
  m_booking = booking;
  
  if (originalEditedBooking)
    m_originalEditedBooking = originalEditedBooking;

  updateTemplate("Deutsch");

  fade();
}

void AdminMailDialog::updateTexts()
{
  ui->okButton->setText(TextHandler::getText(ID::MAIL_ADMIN_SEND));
  ui->cancelButton->setText(TextHandler::getText(ID::MAIL_ADMIN_NO_SEND));
}

void AdminMailDialog::on_okButton_clicked()
{
  MailHandler::sendMail(
    EMailType::AdminEmail, m_booking, 
    ui->mailSubjectLineEdit->text(), ui->mailBodyTextEdit->toPlainText()
  );

  fade();
}
  
void AdminMailDialog::on_cancelButton_clicked()
{
  fade();
}

void AdminMailDialog::on_languageButton_clicked()
{
  const QString& buttonText = ui->languageButton->text();

  if (buttonText == "English") {
     locale = QLocale::English;
     ui->languageButton->setText("Deutsch");
  }
   
  if (buttonText == "Deutsch") {
    locale = QLocale::German;
    ui->languageButton->setText("English");
  }
    
  updateTemplate(buttonText);
}

void AdminMailDialog::updateTemplate(const QString& language)
{
  const QString& formatGerman = TextHandler::getTextGerman(ID::DATE_FORMAT);
  const QString& formatEnglish = TextHandler::getTextEnglish(ID::DATE_FORMAT);

  switch (m_type) {
    case EAdminMailType::Adjustment:
      if (language == "English") {
        ui->mailSubjectLineEdit->setText(
          TextHandler::getTextEnglish(ID::MAIL_ADMIN_ADJUSTMENT_SUBJECT)
        );

        ui->mailBodyTextEdit->setText(
          TextHandler::getTextEnglish(ID::MAIL_ADMIN_ADJUSTMENT_BODY)
            .arg(m_booking->event)
            .arg(locale.toString(m_originalEditedBooking->date, formatEnglish))
            .arg(m_originalEditedBooking->startTime.toString("HH:mm"))
            .arg(m_originalEditedBooking->stopTime.toString("HH:mm"))
            .arg(locale.toString(m_booking->date))
            .arg(m_booking->startTime.toString("HH:mm"))
            .arg(m_booking->stopTime.toString("HH:mm")) +
          TextHandler::getTextEnglish(ID::MAIL_NO_ANSWER)
            .arg(MailHandler::adminEmail)
        );
      }

      if (language == "Deutsch") {
        ui->mailSubjectLineEdit->setText(
          TextHandler::getTextGerman(ID::MAIL_ADMIN_ADJUSTMENT_SUBJECT)
        );

        ui->mailBodyTextEdit->setText(
          TextHandler::getTextGerman(ID::MAIL_ADMIN_ADJUSTMENT_BODY)
            .arg(m_booking->event)
            .arg(locale.toString(m_originalEditedBooking->date, formatGerman))
            .arg(m_originalEditedBooking->startTime.toString("HH:mm"))
            .arg(m_originalEditedBooking->stopTime.toString("HH:mm"))
            .arg(locale.toString(m_booking->date))
            .arg(m_booking->startTime.toString("HH:mm"))
            .arg(m_booking->stopTime.toString("HH:mm")) +
          TextHandler::getTextGerman(ID::MAIL_NO_ANSWER)
            .arg(MailHandler::adminEmail)
        );
      }
    break;

    case EAdminMailType::Deletion:
      if (language == "English") {
        ui->mailSubjectLineEdit->setText(
          TextHandler::getTextEnglish(ID::MAIL_ADMIN_DELETION_SUBJECT)
        );

        ui->mailBodyTextEdit->setText(
          TextHandler::getTextEnglish(ID::MAIL_ADMIN_DELETION_BODY)
            .arg(m_booking->event)
            .arg(locale.toString(m_booking->date, formatEnglish))
            .arg(m_booking->startTime.toString("HH:mm"))
            .arg(m_booking->stopTime.toString("HH:mm")) +
          TextHandler::getTextEnglish(ID::MAIL_NO_ANSWER)
            .arg(MailHandler::adminEmail)
        );
      }

      if (language == "Deutsch") {
        ui->mailSubjectLineEdit->setText(
          TextHandler::getTextGerman(ID::MAIL_ADMIN_DELETION_SUBJECT)
        );

        ui->mailBodyTextEdit->setText(
          TextHandler::getTextGerman(ID::MAIL_ADMIN_DELETION_BODY)
            .arg(m_booking->event)
            .arg(locale.toString(m_booking->date, formatGerman))
            .arg(m_booking->startTime.toString("HH:mm"))
            .arg(m_booking->stopTime.toString("HH:mm")) +
          TextHandler::getTextGerman(ID::MAIL_NO_ANSWER)
            .arg(MailHandler::adminEmail)
        );
      }
    break;
  }
}
