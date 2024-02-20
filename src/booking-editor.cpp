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
  
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
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

    booking.email = bookingToEdit->email;
    booking.index = bookingToEdit->index;
  }

  else {
    isEditing = false;

    QDate currentDate = QDate::currentDate();

    calendarWidget->setSelectedDate(currentDate);
    updateExistingBookingsLabel(currentDate);
    startTimeEdit->setTime(startTimeEdit->minimumTime());
    stopTimeEdit->setTime(stopTimeEdit->minimumTime());
    eventTypeLineEdit->setText("");

    booking.email = BookingManager::getInstance() ? BookingManager::getInstance()->getCurrentMailAddress() : "";
    booking.index = JsonParser::availableIndex();
  }

  booking.date = calendarWidget->selectedDate();
  booking.startTime = startTimeEdit->time();
  booking.stopTime = stopTimeEdit->time();
  booking.event = eventTypeLineEdit->text();

  bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);

  updateExistingBookingsLabel(booking.date);
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

  if (bookingsOnSelectedDate.isEmpty() ||
     (bookingsOnSelectedDate.size() == 1 && bookingsOnSelectedDate[0].index == booking.index)) {
        bookingsOnSelectedDateLabel->setText("There are no bookings yet on " + date.toString() + ".");
        return;
  }

  QString str = "The following times have been booked on " + date.toString() + ":\n";

  str += "<html><head/><body>";

  for (Booking& b : bookingsOnSelectedDate) {
    if (b.index == booking.index) continue;

    bool isConflicting = false;
    
    if (bookingsAreConflicting(booking, b)) 
      isConflicting = true; 
          
    if (isConflicting)
      str += "<span style=\"background-color:red;\">";

    str += b.startTime.toString("HH:mm") + "-" + b.stopTime.toString("HH:mm") + ": " + b.event + " (" + b.email + ")";

    if (isConflicting)
      str += " CONFLICTING!</span>";

    str += "<br/>";
  }

  str += "</body></html>";

  bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  bookingsOnSelectedDateLabel->setText(str);
}

void BookingEditor::updateConflictingBookings(const QDate& date)
{
  QVector<Booking> bookingsOnDate;

  JsonParser::getBookingsOnDate(date, bookingsOnDate);

  for (Booking& b : bookingsOnDate) {
    if (bookingsAreConflicting(booking, b)) {
      booking.isConflicting = true;
      b.isConflicting = true;
    }

    else {
      b.isConflicting = false;
    }

    b.date = date;

    JsonParser::updateBooking(b);
  }
}

bool BookingEditor::bookingsAreConflicting(const Booking& booking0, const Booking& booking1)
{
  if (booking0.index == booking1.index) return false;

  if ((booking0.startTime >= booking1.startTime && booking0.startTime < booking1.stopTime) ||
      (booking0.stopTime > booking1.startTime && booking0.stopTime <= booking1.stopTime))
    return true;

  return false; 
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

  if (time > booking.stopTime) 
    stopTimeEdit->setTime(time);

  updateExistingBookingsLabel(booking.date);
}

void BookingEditor::on_stopTimeEdit_timeChanged(QTime time)
{
  booking.stopTime = time;

  if (time < booking.startTime) 
    startTimeEdit->setTime(time);

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

  if (startTimeEdit->time() == stopTimeEdit->time()) {
    OkDialog::instance("Start and stop time are identical. Please select a time frame!");
    return;
  }
  
  updateConflictingBookings(booking.date);

  if (isEditing) 
    JsonParser::updateBooking(booking);
  
  else 
    JsonParser::addBooking(booking);

  bookingManager->loadBookings();

  hide();
}

void BookingEditor::on_cancelButton_pressed()
{
  hide();
}