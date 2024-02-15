#include <obs-module.h>
#include <obs-frontend-api.h>
#include "login.hpp"
#include "ui_login.h"
#include "ptz.h"
#include "ptz-controls.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"

Login* Login::instance = NULL;

void login_load(void)
{
  QWidget* main_window =
    (QWidget*)obs_frontend_get_main_window();
  auto* tmp = new Login(main_window);
  obs_frontend_add_dock(tmp);
  tmp->setFloating(false);
}

Login::Login(QWidget *parent)
  : QDockWidget("Login", parent), 
    ui(new Ui::Login), 
    mailAddressIsValid(false)
{
  instance = this;
  ui->setupUi(this);
  setAllowedAreas(Qt::RightDockWidgetArea);
  setFeatures(QDockWidget::NoDockWidgetFeatures);
  setTitleBarWidget(new QWidget());

  QLabel* reminderLabel = findChild<QLabel*>("reminderLabel");
  reminderLabel->setMinimumHeight(reminderLabel->fontMetrics().lineSpacing() * 2);

  show();
}

Login::~Login()
{
    delete ui;
}

bool Login::verifyMailAddress()
{
  if (!mailAddressIsValid) {
    findChild<QLabel*>("reminderLabel")->setText("Please enter a valid HfMDD email address");
    findChild<QLineEdit*>("mailAddressLineEdit")->setStyleSheet("QLineEdit { border: 2px solid #FF5952 }");
    return false;
  }

  else {
    findChild<QLabel*>("reminderLabel")->setText(" ");
    findChild<QLineEdit*>("mailAddressLineEdit")->setStyleSheet("QLineEdit { border: 2px solid #48FF8B }");
  }

  return true;
}

void Login::on_mailAddressLineEdit_textEdited(const QString& text)
{
  currentMailAddress = text;
  mailAddressIsValid = false;

  const QString mailSuffices[] = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net" }; // TODO: Remove gmx
  
  for (const QString& suffix : mailSuffices)
  {
    if (currentMailAddress.endsWith(suffix))
    {
      mailAddressIsValid = true;
      innerDir = currentMailAddress.chopped(suffix.length());
      break;
    }
  }

  if (currentMailAddress == "oliver.fenk@hfmdd.de")
    PasswordDialog::instance(mailAddressIsValid, this);

  verifyMailAddress();
}

void Login::on_manageBookingsButton_pressed()
{
  if (!verifyMailAddress()) return;

  BookingManager* bookingManager = BookingManager::getInstance();

  if (bookingManager) {
    // OkDialog::instance("Booking Manager instance valid", this);
    bookingManager->loadBookings();
    bookingManager->show();
  }
    

  else {
    // OkDialog::instance("Booking Manager instance invalid", this);
    bookingManager = new BookingManager(this);

    if (!bookingManager) return;
    
    bookingManager->exec();
  }
}

void Login::on_toPTZControlsButton_pressed()
{
  if (!verifyMailAddress()) return;

  hide();
  PTZControls::getInstance()->setFloating(false);
  PTZControls::getInstance()->show();
}