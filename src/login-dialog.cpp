#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>
#include <QPainter>
#include <QPixmap>

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
    mailAddressIsValid(false),
    reminderLabelText("Please log in with your HfMDD email address! Only admin accounts require a password."),
    passwordLineEditPlaceholderText("Password")
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");

  instance = this;
  ui->setupUi(this);
  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #f21a1a }");
  ui->backgroundWidget->setStyleSheet("background-image: url(C:/dev/test-background.png);");
  // ui->logoWidget->setStyleSheet("background: transparent;");
  ui->logoWidget->setStyleSheet("background-image: url(C:/dev/logo-white.png);"
                                "background-repeat: no-repeat;"
                                "background-position: center;"
                                "background-size: contain;");
  ui->nameWidget->setStyleSheet("background-image: url(C:/dev/name.png);"
                                "background-repeat: no-repeat;"
                                "background-position: center;"
                                "background-size: contain;");
  // ui->manageBookingsButton->setStyleSheet("QPushButton:focus { background-color: #f21a1a }"
  //                        "QPushButton:focus:pressed { background-color: #f21a1a }");

  ui->languageComboBox->addItem("Deutsch");
  ui->languageComboBox->addItem("English");
  ui->languageComboBox->setCurrentText("English");

  ui->languageComboBox->setStyleSheet("QComboBox { background-color: #ffbf00; border-radius: none; color: rgb(31, 30, 31); font-size: 16px; }");
  // ui->languageComboBox->view().setStyleSheet("QListView { background-color: #f21a1a; }");
  QListView* dropdown = static_cast<QListView*>(ui->languageComboBox->view());
  dropdown->setStyleSheet("QListView { background-color: #ffbf00; color: rgb(31, 30, 31); font-size: 16px; }"
                          "QListView::item { min-height: 64px; }"
                          "QListView::item:hover { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: none; }");

  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border-radius: none; }");
  ui->passwordLineEdit->setStyleSheet("QLineEdit { border-radius: none; }");

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
  QPoint nameWidgetPos = QPoint(180, 24);
  QPoint languageComboBoxPos = QPoint(
    screenWidth - (ui->languageComboBox->width()), 
    16 + ui->languageComboBox->height()
  );

  ui->logoWidget->move(logoWidgetPos);
  ui->nameWidget->move(nameWidgetPos);
  ui->languageComboBox->move(languageComboBoxPos);

  ui->backgroundWidget->setFixedWidth(160);
  ui->backgroundWidget->setFixedHeight(screenHeight);

  ui->reminderLabelWidget->move(180, screenHeight - (8 + ui->reminderLabelWidget->height()));
  
  ui->mailAddressLineEdit->clear();
  ui->passwordLineEdit->clear();
}

void LoginDialog::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    reminderLabelText = "Bitte loggen Sie sich mit Ihrer HfMDD-E-Mail-Adresse ein! Nur Administratorkonten erfordern ein Passwort.";
    passwordLineEditPlaceholderText = "Passwort";
    break;

    case ELanguage::English:
    reminderLabelText = "Please log in with your HfMDD email address! Only admin accounts require a password.";
    passwordLineEditPlaceholderText = "Password";
    break;
  }

  ui->reminderLabel->setText(reminderLabelText);
  ui->passwordLineEdit->setPlaceholderText(passwordLineEditPlaceholderText);
}

bool LoginDialog::verifyMailAddress()
{
  if (!mailAddressIsValid) {
    ui->reminderLabel->show();
    ui->reminderLabel->setText(reminderLabelText);
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #f21a1a; border-radius: none; background-color: rgb(254, 253, 254)}");
    return false;
  }

  else {
    ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid rgb(31, 30, 31); border-radius: none; background-color: rgb(254, 253, 254)}");
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

    else ui->reminderLabel->setText(reminderLabelText);
  }

  if (Backend::currentEmail == Backend::adminEmail) 
    ui->passwordLineEdit->setDisabled(false);

  else 
    ui->passwordLineEdit->setDisabled(true);

  QFont font = ui->mailAddressLineEdit->font();

  if (text == "")
    ui->mailAddressLineEdit->setFont(QFont(font.family(), font.pointSize(), font.weight(), true));

  else 
    ui->mailAddressLineEdit->setFont(QFont(font.family(), font.pointSize(), font.weight(), false));

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

void LoginDialog::on_languageComboBox_currentTextChanged(const QString& text)
{
  if (text == "English") {
    Backend::language = ELanguage::English;

    for (Translatable* t : Widgets::translatables)
        t->translate(ELanguage::English);
  }

  if (text == "Deutsch") {
    Backend::language = ELanguage::German;

    for (Translatable* t : Widgets::translatables)
      t->translate(ELanguage::German);
  }
}