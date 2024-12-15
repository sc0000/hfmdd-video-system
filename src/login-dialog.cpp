/* Full screen login widget 
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>
#include <QPainter>
#include <QPixmap>
#include <QDir>

#include "booking-handler.hpp"
#include "mail-handler.hpp"
#include "widgets.hpp"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "mode-select.hpp"
#include "styles.hpp"
#include "ui_login-dialog.h"
#include "login-dialog.hpp"

LoginDialog::LoginDialog(QWidget *parent)
  : FullScreenDialog(parent),
    ui(new Ui::LoginDialog),
    reminderLabelText("Please log in with your HfMDD email address!"),
    infoDialogText("")
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");
  setFont(QFont("DaxOT-Medium"));

  ui->setupUi(this);
  ui->mailAddressLineEdit->setStyleSheet(Styles::invalidLineEdit);

  ui->reminderLabel->setText(reminderLabelText);
  ui->reminderLabel->hide();
  
  // Construct the stylesheet strings with actual values
  QString backgroundWidgetStyleSheet = QString("background-image: url(../../assets/sidebar.png);");
  ui->backgroundWidget->setStyleSheet(backgroundWidgetStyleSheet);

  QString logoWidgetStyleSheet = QString("background-image: url(../../assets/logo-white.png);"
                                        "background-repeat: no-repeat;"
                                        "background-position: center;"
                                        "background-size: contain;");
  ui->logoWidget->setStyleSheet(logoWidgetStyleSheet);

  QString nameLabelStyleSheet = QString("QLabel { color: %1; }").arg(Color::yellow);
  ui->nameLabel->setStyleSheet(nameLabelStyleSheet);

  ui->languageComboBox->addItem("Deutsch");
  ui->languageComboBox->addItem("English");
  ui->languageComboBox->setCurrentText("English");

  ui->languageComboBox->setStyleSheet(QString(
    "QComboBox { background-color: %1; border-radius: none; color: %2; font-size: 16px; }"
    "QComboBox::down-arrow { qproperty-alignment: AlignTop; image: url(../../assets/down-black.svg); width: 100%;}"
 ).arg(Color::yellow).arg(Color::black));

  QListView* dropdown = static_cast<QListView*>(ui->languageComboBox->view());
  dropdown->setStyleSheet(QString(
    "QListView { background-color: %1; color: %2; font-size: 16px; }"
    "QListView::item { min-height: 64px; }"
    "QListView::item:hover { background-color: %2; color: %3; border: none; }"
  ).arg(Color::yellow).arg(Color::black).arg(Color::white));

  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border-radius: none; }");

  ui->infoButton->setStyleSheet(Styles::infoButtonOff);

  updateTexts();

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
  center(ui->masterWidget, 50, 0);

   QWidget* mainWindow = (QWidget*)obs_frontend_get_main_window();

  if (!mainWindow) return;

  QRect screenGeometry = mainWindow->screen()->geometry();

  int screenWidth = screenGeometry.width();
  int screenHeight = screenGeometry.height();

  QPoint logoWidgetPos = QPoint(20, 20);
  QPoint nameLabelPos = QPoint(180, 8);
  QPoint languageComboBoxPos = QPoint(
    screenWidth - (ui->languageComboBox->width()), 
    24 + ui->languageComboBox->height()
  );

  QPoint infoButtonPos = QPoint(
    screenWidth - (ui->infoButton->width() + 20),
    screenHeight - (ui->infoButton->height() + 20)
  );

  ui->logoWidget->move(logoWidgetPos);
  ui->nameLabel->move(nameLabelPos);
  ui->languageComboBox->move(languageComboBoxPos);
  ui->infoButton->move(infoButtonPos);

  ui->backgroundWidget->setFixedWidth(160);
  ui->backgroundWidget->setFixedHeight(screenHeight);

  ui->reminderLabelWidget->move(180, screenHeight - (8 + ui->reminderLabelWidget->height()));
  
  ui->mailAddressLineEdit->clear();
}

void LoginDialog::updateTexts()
{
  ui->reminderLabel->setText(TextHandler::getText(ID::LOGIN_REMINDER));
  infoDialogText = TextHandler::getText(ID::LOGIN_INFO);
}

bool LoginDialog::verifyMailAddress()
{
  if (!MailHandler::mailAddressIsValid) {
    ui->reminderLabel->setText(reminderLabelText);
    ui->mailAddressLineEdit->setStyleSheet(Styles::invalidLineEdit);

    return false;
  }

  else 
    ui->mailAddressLineEdit->setStyleSheet(Styles::validLineEdit);

  return true;
}

void LoginDialog::on_mailAddressLineEdit_textChanged(const QString& text)
{
  MailHandler::currentEmail = text;
  MailHandler::mailAddressIsValid = false;

  for (const QString& suffix : MailHandler::mailSuffices)
  {
    if (MailHandler::currentEmail.endsWith(suffix))
    {
      MailHandler::mailAddressIsValid = true;
      break;
    }
  }

  QFont font = ui->mailAddressLineEdit->font();

  if (text == "")
    ui->mailAddressLineEdit->setFont(QFont(font.family(), font.pointSize(), font.weight(), true));

  else 
    ui->mailAddressLineEdit->setFont(QFont(font.family(), font.pointSize(), font.weight(), false));

  verifyMailAddress();
}

void LoginDialog::on_manageBookingsButton_clicked()
{
  if (!verifyMailAddress()) {
    Widgets::okDialog->display(TextHandler::getText(ID::LOGIN_REMINDER));
    return;
  }

  if (MailHandler::currentEmail == MailHandler::adminEmail) {
    int result = Widgets::passwordDialog->display();

    if (result == QDialog::Rejected)    
      return;

    MailHandler::isAdmin = true; 
  }

  else MailHandler::isAdmin = false; 

  fade(Widgets::modeSelect);
}

void LoginDialog::on_languageComboBox_currentTextChanged(const QString& text)
{
  TextHandler::translate(text);

  for (Translatable* t : Widgets::translatables)
    t->updateTexts();
}

void LoginDialog::on_infoButton_pressed()
{
  Widgets::infoDialog->display(TextHandler::getText(ID::LOGIN_INFO).arg(MailHandler::adminEmail), ui->infoButton);
}
