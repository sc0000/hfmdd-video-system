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
    bookingManager->loadBookings();
    bookingManager->show();
  }
    

  else {
    bookingManager = new BookingManager(this);

    if (!bookingManager) return;
    
    bookingManager->exec();
  }
}

static void item_select_cb(void* data, calldata_t* cd) {
  // obs_source_t* scene = (obs_source_t*)calldata_ptr(cd, "scene");
  obs_sceneitem_t* item = (obs_sceneitem_t*)calldata_ptr(cd, "item");
  
  // actual useful code...


  obs_source_t* source = obs_sceneitem_get_source(item); 

  PTZControls::getInstance()->currCameraName = QString(obs_source_get_name(source));

  // OkDialog::instance(QString(obs_source_get_name(source)));
}

void Login::on_toPTZControlsButton_pressed()
{
  if (!verifyMailAddress()) return;

  hide();
  PTZControls::getInstance()->setFloating(false);
  PTZControls::getInstance()->show();

  obs_source_t* scene_source = obs_frontend_get_current_scene();
  signal_handler_t *sh = obs_source_get_signal_handler(scene_source);
  signal_handler_connect(sh, "item_select", item_select_cb, NULL);
}