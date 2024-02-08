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
   addBooking(b);
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

  if (bookingsList != nullptr)
    bookingsList->addItem(makeEntry(booking));
}

void BookingManager::updateBooking(const Booking& booking)
{
  int rowIndex = bookingsList->currentRow();

  bookings[rowIndex] = booking;

  QListWidgetItem* item = bookingsList->takeItem(rowIndex);
  if (item) delete item;

  bookingsList->insertItem(rowIndex, makeEntry(booking));
  JsonParser::updateBooking(rowIndex, booking);
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