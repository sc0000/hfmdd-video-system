#include <QLabel>
#include "login.hpp"
#include "message-box.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"
#include "json-parser.hpp"


BookingManager* BookingManager::instance = NULL;

BookingManager::BookingManager(QWidget* parent)
  : QDialog(parent), 
    ui(new Ui::BookingManager),
    currentMailAddress(Login::getInstance()->getCurrentMailAddress())
{
  setWindowTitle("Booking Manager");
  // setWindowFlags(Qt::WindowTitleHint | Qt::CustomizeWindowHint);
  instance = this;
  ui->setupUi(this);
  bookingsList = findChild<QListWidget*>("bookingsList");
  
  updateBookings();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::updateBookings()
{
  bookings.clear();

  JsonParser::getBookingsForEmail(currentMailAddress, bookings);

  for (const Booking& b : bookings) {
    QString entry = 
      b.date.toString() + ", " +
      b.startTime.toString("HH:mm") + " - " +
      b.stopTime.toString("HH:mm") + ": " +
      b.event;

    bookingsList->addItem(entry);
  }
}

void BookingManager::on_newBookingButton_pressed()
{
  Login* l = Login::getInstance();
  l->findChild<QLabel*>("reminderLabel")->setText("TEST");
  // MessageBox::dInstance("test", nullptr);
  // delete this;
}

void BookingManager::on_editBookingButton_pressed()
{
  
}

void BookingManager::on_deleteBookingButton_pressed()
{
  int rowIndex = bookingsList->currentRow();
  const Booking& selectedBooking = bookings[rowIndex];
  JsonParser::removeBooking(selectedBooking);
  bookings.removeAt(rowIndex);
  QListWidgetItem* item = bookingsList->takeItem(rowIndex);
  if (item) delete item;
}