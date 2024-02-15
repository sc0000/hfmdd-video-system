#include <QLabel>
#include "login.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "ui_booking-manager.h"
#include "booking-manager.hpp"
#include "json-parser.hpp"


BookingManager* BookingManager::instance = NULL;

BookingManager::BookingManager(QWidget* parent)
  : QDialog(parent), 
    ui(new Ui::BookingManager)
{
  if (parent)
    currentMailAddress = static_cast<Login*>(parent)->getCurrentMailAddress();

  setWindowTitle("Booking Manager");
  instance = this;
  ui->setupUi(this);
  bookingsList = findChild<QListWidget*>("bookingsList");
  
  loadBookings();
}

BookingManager::~BookingManager()
{
  delete ui;
}

void BookingManager::loadBookings()
{
  QVector<Booking> temp;

  JsonParser::getBookingsForEmail(currentMailAddress, temp);

  for (const Booking& b : temp) {
   bookings.append(b);
  }

  sortBookings();

  if (!bookingsList) return;

  bookingsList->clear();
    
  for (const Booking& b : bookings)
    bookingsList->addItem(makeEntry(b));
}

void BookingManager::sortBookings()
{
  qsizetype size = bookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = bookings[j].date.toString(Qt::ISODate) + "_" +
        bookings[j].startTime.toString("HH:mm");

      QString dateTime1 = bookings[j + 1].date.toString(Qt::ISODate) + "_" +
        bookings[j + 1].startTime.toString("HH:mm");

      if (dateTime0 > dateTime1) {
        Booking tmp = bookings[j];
        bookings[j] = bookings[j + 1];
        bookings[j + 1] = tmp;

        flag = true;
      }
    }

    if (flag == 0) break;
  }


}

QString BookingManager::makeEntry(const Booking& booking)
{
  QString entry = 
      booking.date.toString() + ", " +
      booking.startTime.toString("HH:mm") + " - " +
      booking.stopTime.toString("HH:mm") + ": " +
      booking.event + 
      " (" + booking.email + ")";

  return entry;
}

void BookingManager::addBooking(const Booking& booking)
{
  bookings.append(booking);
  sortBookings();

  if (!bookingsList) return;

  bookingsList->clear();
    
  for (const Booking& b : bookings)
    bookingsList->addItem(makeEntry(b));
}

void BookingManager::updateBooking(const Booking& booking)
{
  int rowIndex = bookingsList->currentRow();

  bookings[rowIndex] = booking;
  sortBookings();

  JsonParser::updateBooking(rowIndex, booking);

  if (!bookingsList) return;
  
  bookingsList->clear();
  
  for (const Booking& b : bookings)
    bookingsList->addItem(makeEntry(b));
}

void BookingManager::on_newBookingButton_pressed()
{
  BookingEditor::instance(nullptr, this);
}

void BookingManager::on_editBookingButton_pressed()
{
  BookingEditor::instance(&bookings[bookingsList->currentRow()], this);
}

void BookingManager::on_deleteBookingButton_pressed()
{
  if (bookingsList->selectedItems().isEmpty()) {
    OkDialog::instance("Please select the booking you want to delete.", this);
    return;
  }

  bool confirmed;

  OkCancelDialog::instance("Do you really want to delete the selected booking? This cannot be undone.", confirmed, this);

  if (!confirmed) return;

  int rowIndex = bookingsList->currentRow();
  const Booking& selectedBooking = bookings[rowIndex];
  JsonParser::removeBooking(selectedBooking);
  bookings.removeAt(rowIndex);
  QListWidgetItem* item = bookingsList->takeItem(rowIndex);
  if (item) delete item;
}