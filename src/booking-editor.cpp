#include <QLabel>
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "ui_booking-editor.h"
#include "booking-editor.hpp"

BookingEditor::BookingEditor(Booking* bookingToEdit, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::BookingEditor)
{
  ui->setupUi(this);

   setWindowTitle("Booking Editor");
  
  calendarWidget = findChild<QCalendarWidget*>("calendarWidget");
  bookingsOnSelectedDateLabel = findChild<QLabel*>("bookingsOnSelectedDateLabel");
  startTimeEdit = findChild<QTimeEdit*>("startTimeEdit");
  stopTimeEdit = findChild<QTimeEdit*>("stopTimeEdit");
  eventTypeLineEdit = findChild<QLineEdit*>("eventTypeLineEdit");

  if (bookingToEdit) {
    isEditing = true;

    calendarWidget->setSelectedDate(bookingToEdit->date);
    updateExistingBookingsLabel(bookingToEdit->date);

    startTimeEdit->setTime(bookingToEdit->startTime);
    stopTimeEdit->setTime(bookingToEdit->stopTime);
    eventTypeLineEdit->setText(bookingToEdit->event);
  }

  else {
    isEditing = false;

    QDate currentDate = QDate::currentDate();

    calendarWidget->setSelectedDate(currentDate);
    updateExistingBookingsLabel(currentDate);
    startTimeEdit->setTime(startTimeEdit->minimumTime());
    stopTimeEdit->setTime(stopTimeEdit->minimumTime());
    eventTypeLineEdit->setText("");
  }

  booking.email = BookingManager::getInstance() ? BookingManager::getInstance()->getCurrentMailAddress() : "";
  booking.date = calendarWidget->selectedDate();
  booking.startTime = startTimeEdit->time();
  booking.stopTime = stopTimeEdit->time();
  booking.event = eventTypeLineEdit->text();

  bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
}

BookingEditor::~BookingEditor()
{
  delete ui;
}

void BookingEditor::instance(Booking* bookingToEdit, QWidget* parent)
{
  std::unique_ptr<BookingEditor> bookingEditor = std::make_unique<BookingEditor>(bookingToEdit, parent);
  bookingEditor->exec();
}

void BookingEditor::updateExistingBookingsLabel(QDate date)
{
  bookingsOnSelectedDate.clear();
  
  JsonParser::getBookingsOnDate(date, bookingsOnSelectedDate);

  if (bookingsOnSelectedDate.isEmpty())
  {
    bookingsOnSelectedDateLabel->setText("There are no bookings yet on " + date.toString() + ".");
    return;
  }

  QString str = "The following times have been booked on " + date.toString() + ":\n";

  str += "<html><head/><body>";

  for (const Booking& b : bookingsOnSelectedDate) {
    if ((booking.startTime >= b.startTime && booking.startTime < b.stopTime) ||
        (booking.stopTime > b.startTime && booking.stopTime <= b.stopTime))
        str += "<span style=\"background-color:red;\">";

    str += b.startTime.toString() + "-" + b.stopTime.toString() + ": " + b.event + " (" + b.email + ")";

    if ((booking.startTime >= b.startTime && booking.startTime < b.stopTime) ||
        (booking.stopTime > b.startTime && booking.stopTime <= b.stopTime))
        str += " COLLIDING!</span>";

    str += "<br/>";
  }

  str += "</body></html>";

  bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  bookingsOnSelectedDateLabel->setText(str);
}

void BookingEditor::on_calendarWidget_clicked(QDate date)
{
  booking.date = date;

  if (!bookingsOnSelectedDateLabel) return;

  updateExistingBookingsLabel(date);
}

void BookingEditor::on_startTimeEdit_timeChanged(QTime time)
{
  booking.startTime = time;
  updateExistingBookingsLabel(booking.date);
}

void BookingEditor::on_stopTimeEdit_timeChanged(QTime time)
{
  booking.stopTime = time;
  updateExistingBookingsLabel(booking.date);
}

void BookingEditor::on_eventTypeLineEdit_textChanged(const QString& text)
{
  booking.event = text;
}

void BookingEditor::on_saveButton_pressed()
{
  BookingManager* bookingManager = BookingManager::getInstance();

  if (!bookingManager) 
    return;

  if (booking.event == "") {
    OkDialog::instance("Please specify the type of event!");
    return;
  }

  if (isEditing) {
    bookingManager->updateBooking(booking);
  }

  else {
    JsonParser::addBooking(booking);
    bookingManager->addBooking(booking);
  }

  hide();
}

void BookingEditor::on_cancelButton_pressed()
{
  hide();
}