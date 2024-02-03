#include <QLabel>
#include "json-parser.hpp"
#include "ui_booking-editor.h"
#include "booking-editor.hpp"

BookingEditor::BookingEditor(Booking* bookingToEdit, QWidget* parent)
  : QDialog(parent),
    ui(new Ui::BookingEditor)
{
  ui->setupUi(this);
  existingBookingsLabel = findChild<QLabel*>("existingBookingsLabel");

  if (bookingToEdit)
    booking = *bookingToEdit;
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

void BookingEditor::on_calendarWidget_clicked(QDate date)
{
  booking.date = date;

  if (!existingBookingsLabel) return;

  QVector<Booking> existingBookings;
  
  JsonParser::getBookingsOnDate(date, existingBookings);

  if (existingBookings.isEmpty())
  {
    existingBookingsLabel->setText("There are no bookings yet on " + date.toString() + ".");
    return;
  }

  QString str = "The following times have been booked on " + date.toString() + ":\n";

  for (const Booking& b : existingBookings)
    str += b.startTime.toString() + "-" + b.stopTime.toString() + ": " + b.event + " (" + b.email + ")\n";

  existingBookingsLabel->setText(str);
}

void BookingEditor::on_startTimeEdit_timeChanged(QTime time)
{

}

void BookingEditor::on_stopTimeEdit_timeChanged(QTime time)
{

}

void BookingEditor::on_eventTypeLineEdit_textChanged(const QString& text)
{

}

void BookingEditor::on_saveButton_pressed()
{

}

void BookingEditor::on_cancelButton_pressed()
{
  hide();
}