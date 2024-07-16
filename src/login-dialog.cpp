#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QScreen>
#include <QPainter>
#include <QPixmap>
#include <QDir>

#include "backend.hpp"
#include "widgets.hpp"
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
    reminderLabelText("Please log in with your HfMDD email address!"),
    passwordLineEditPlaceholderText("Only admin accounts require a password."),
    infoDialogText("")
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  setWindowTitle("Login Dialog");
  setFont(QFont("DaxOT-Medium"));

  instance = this;
  ui->setupUi(this);
  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border: 2px solid #f21a1a }");

  infoDialogText = QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" +
      "This is a system with which every member of HfMDD can independently record their concert hall performances on video.<br/><br/>" +
      "<span style=\"font-weight: bold;\">How does it work?</span><br/>First, log in with your HfMDD email. Only admin accounts require a password.<br/>" +
      "Then, you have two options: You could make a recording right away, only setting the stop time.<br/>"
      "However, you could also work with the booking system, and reserve a time slot to record a video at a later time or date.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Anything to be aware of?</span><br/>" +
      "No matter which route you'll take, the system will let you know of any other planned recordings that might be in conflict with what you're planning.<br/>" +
      "As of yet, you are not technically prohibited to make conflicting bookings, or let a recording run into another booked timeslot.<br/>" +
      "Therefore, it is all the more important to communicate with whomever you're colliding with. Conflicting bookings might also be reviewed by the admin, and, if needed, adjusted.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Who can I talk to about this?</span><br/>For questions and feedback, please contact Oliver Fenk (oliver.fenk@hfmdd.de)." +
      "</body></html>";

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

  QString nameLabelStyleSheet = QString("QLabel { color: #ffbf00; }");
  ui->nameLabel->setStyleSheet(nameLabelStyleSheet);

  ui->languageComboBox->addItem("Deutsch");
  ui->languageComboBox->addItem("English");
  ui->languageComboBox->setCurrentText("English");

  ui->languageComboBox->setStyleSheet("QComboBox { background-color: #ffbf00; border-radius: none; color: rgb(31, 30, 31); font-size: 16px; }");
  QListView* dropdown = static_cast<QListView*>(ui->languageComboBox->view());
  dropdown->setStyleSheet("QListView { background-color: #ffbf00; color: rgb(31, 30, 31); font-size: 16px; }"
                          "QListView::item { min-height: 64px; }"
                          "QListView::item:hover { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: none; }");

  ui->mailAddressLineEdit->setStyleSheet("QLineEdit { border-radius: none; }");
  ui->passwordLineEdit->setStyleSheet("QLineEdit { border-radius: none; }");

  ui->infoButton->setStyleSheet(
    "QPushButton { color: rgb(254, 253, 254); background-color: rgb(31, 30, 31); }"
    "QPushButton:hover { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
  );

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
  ui->passwordLineEdit->clear();
  // ui->passwordLineEdit->hide();
}

void LoginDialog::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    reminderLabelText = "Bitte loggen Sie sich mit Ihrer HfMDD-E-Mail-Adresse ein!";
    passwordLineEditPlaceholderText = "Nur Administratorenkonten erfordern ein Passwort.";
    infoDialogText = QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">Was ist das hier?</span><br/>" +
      "Das hier ist ein System, mit dem jedes Mitglied der HfMDD seine Konzertsaalauftritte selbstständig auf Video aufzeichnen kann.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Wie funktioniert das?</span><br/>Melden Sie sich zunächst mit Ihrer HfMDD-E-Mail-Adresse an. Nur Administratorkonten erfordern ein Passwort. " +
      "Dann haben Sie zwei Möglichkeiten:<br/>Sie können direkt mit der Aufnahme beginnen und nur die Stoppzeit angeben, "
      "oder das Buchungssystem nutzen und ein Zeitfenster für eine Aufnahme reservieren.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Gibt es etwas, worauf ich achten muss?</span><br/>" +
      "Das System weist Sie auf bereits existierende Buchungen hin, die möglicherweise mit Ihren Plänen kollidieren.<br/>" +
      "Aktuell ist es durchaus möglich, kollierende Zeitfenster zu buchen, wie auch in reservierte Zeitfenster hinein aufzunehmen. " +
      "In solchen Fällen ist es ratsam, ggf. den Kontakt zur betreffenden Person zu suchen. Kollidierende Buchungen werden außerdem vom Administrator überprüft und Umständen entsprechend angepasst.<br/><br/>" +
      "<span style=\"font-weight: bold;\">An wen kann ich mich bei Fragen und Problemen wenden?</span><br/>Oliver Fenk (oliver.fenk@hfmdd.de)." +
      "</body></html>";
    break;

    case ELanguage::English:
    reminderLabelText = "Please log in with your HfMDD email address!";
    passwordLineEditPlaceholderText = "Only admin accounts require a password.";
    infoDialogText = QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" +
      "This is a system with which every member of HfMDD can independently record their concert hall performances on video.<br/><br/>" +
      "<span style=\"font-weight: bold;\">How does it work?</span><br/>First, log in with your HfMDD email. Only admin accounts require a password.<br/>" +
      "Then, you have two options: You could make a recording right away, only setting the stop time.<br/>"
      "However, you could also work with the booking system, and reserve a time slot to record a video at a later time or date.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Anything to be aware of?</span><br/>" +
      "No matter which route you'll take, the system will let you know of any other planned recordings that might be in conflict with what you're planning.<br/>" +
      "As of yet, you are not technically prohibited to make conflicting bookings, or let a recording run into another booked timeslot.<br/>" +
      "Therefore, it is all the more important to communicate with whomever you're colliding with. Conflicting bookings might also be reviewed by the admin, and, if needed, adjusted.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Who can I talk to about this?</span><br/>For questions and feedback, please contact Oliver Fenk (oliver.fenk@hfmdd.de)." +
      "</body></html>";
    break;
  }

  ui->reminderLabel->setText(reminderLabelText);
  ui->passwordLineEdit->setPlaceholderText(passwordLineEditPlaceholderText);
}

bool LoginDialog::verifyMailAddress()
{
  if (!Backend::mailAddressIsValid) {
    // ui->reminderLabel->show();
    ui->reminderLabel->setText(reminderLabelText);
    ui->mailAddressLineEdit->setStyleSheet(
      "QLineEdit { border: 2px solid #f21a1a; border-radius: 0px; background-color: rgb(254, 253, 254)}"
    );

    return false;
  }

  else {
    ui->mailAddressLineEdit->setStyleSheet(
      "QLineEdit { border: 2px solid rgb(31, 30, 31); border-radius: 0px; background-color: rgb(254, 253, 254)}"
    );
  }

  return true;
}

void LoginDialog::on_mailAddressLineEdit_textChanged(const QString& text)
{
  Backend::currentEmail = text;
  Backend::mailAddressIsValid = false;
  
  // if (Backend::mailAddressIsValid)
  //   ui->reminderLabel->setText("");

  // else 
  //   ui->reminderLabel->setText(reminderLabelText);

  for (const QString& suffix : SettingsManager::mailSuffices)
  {
    if (Backend::currentEmail.endsWith(suffix))
    {
      Backend::mailAddressIsValid = true;
      // ui->reminderLabel->setText("");
      break;
    }

    // else ui->reminderLabel->setText(reminderLabelText);
  }

  if (Backend::currentEmail == Backend::adminEmail) 
    ui->passwordLineEdit->setDisabled(false);
    // ui->passwordLineEdit->show();

  else 
    ui->passwordLineEdit->setDisabled(true);
    // ui->passwordLineEdit->hide();

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
    Widgets::okDialog->display(Backend::language != ELanguage::German ?
      "Please log in with your HfMDD email!" :
      "Bitten loggen Sie sich mit Ihrer HfMDD-E-Mail-Adresse ein!"
    );

    return;
  }

  if (ui->passwordLineEdit->isEnabled() && ui->passwordLineEdit->text() != Backend::adminPassword) {
    Widgets::okDialog->display(Backend::language != ELanguage::German ?
      "The password is incorrect." :
      "Das Passwort ist inkorrekt."
    );

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

void LoginDialog::on_infoButton_pressed()
{
  if (!Widgets::infoDialog->isVisible()) {
    ui->infoButton->setStyleSheet(
      "QPushButton { color: rgb(254, 253, 254); background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    );

    Widgets::infoDialog->display(infoDialogText, ui->infoButton);
  }
    

  else {
    ui->infoButton->setStyleSheet(
      "QPushButton { color: rgb(254, 253, 254); background-color: rgb(31, 30, 31); }"
      "QPushButton:hover { background-color: rgb(42,130,218); border: 1px solid rgb(254, 253, 254); }"
    );

    Widgets::infoDialog->fade();
    // Widgets::infoDialog->hide();
  }
}