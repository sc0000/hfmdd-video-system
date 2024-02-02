#include <QLabel>
#include "login.hpp"
#include "message-box.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"


BookingManager* BookingManager::instance = NULL;

BookingManager::BookingManager(QWidget* parent)
  : QDialog(parent), ui(new Ui::BookingManager)
{
  // setWindowFlag(Qt::FramelessWindowHint);
  instance = this;
  ui->setupUi(this);
}

BookingManager::~BookingManager()
{

}

void BookingManager::on_newBookingButton_pressed()
{
  Login* l = Login::getInstance();
  l->findChild<QLabel*>("reminderLabel")->setText("TEST");
  // MessageBox::dInstance("test", nullptr);
  // delete this;
}

void BookingManager::on_editSelectedBookingButton_pressed()
{

}