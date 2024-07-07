#include <QLabel>

#include "backend.hpp"
#include "widgets.hpp"
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "settings-manager.hpp"
#include "booking-manager.hpp"
#include "ui_booking-editor.h"
#include "booking-editor.hpp"

BookingEditor::BookingEditor(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::BookingEditor),
    booking(Backend::currentBooking),
    timeToSet(ETimeToSet::StartTime)
{
  ui->setupUi(this);

  ui->calendarWidget->setStyleSheet("QCalendarWidget { border: 1px solid rgb(31, 30, 31); }");
  
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
  setWindowTitle("Booking Editor");
}

BookingEditor::~BookingEditor()
{
  delete ui;
}

void BookingEditor::reload(Booking* bookingToEdit)
{
  if (bookingToEdit) {
    isEditing = true;

    ui->eventTypeLineEdit->setText(bookingToEdit->event);

    booking.date = bookingToEdit->date;
    booking.email = bookingToEdit->email;
    booking.index = bookingToEdit->index;
    booking.startTime = bookingToEdit->startTime;
    booking.stopTime = bookingToEdit->stopTime;
  }

  else {
    booking.date = QDate::currentDate();
    booking.email = Backend::currentEmail;
    booking.event = "";
    booking.index = JsonParser::availableIndex();
    booking.startTime = QTime::currentTime();
    Backend::roundTime(booking.startTime);
    booking.stopTime = booking.startTime.addSecs(60 * 60) < QTime(23, 0) ? 
      booking.startTime.addSecs(60 * 60) : 
      QTime(23, 0);

    ui->eventTypeLineEdit->setText("");
  }

  ui->setStartTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px; }");
  ui->calendarWidget->setSelectedDate(booking.date);
  updateExistingBookingsLabel(booking.date);

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);

  drawTimespan();

  exec();
  raise();
}

void BookingEditor::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    ui->bookingsOnSelectedDateLabel->setText("Kein Datum ausgewählt");
    ui->setStartTimeButton->setText("Startzeit ändern");
    ui->setStopTimeButton->setText("Stopzeit ändern");
    ui->eventTypeLabel->setText("Art der Veranstaltung");
    ui->saveButton->setText("Speichern");
    ui->cancelButton->setText("Verwerfen");
    ui->calendarWidget->setLocale(QLocale::German);
    break;

    case ELanguage::English:
    ui->bookingsOnSelectedDateLabel->setText("No date selected.");
    ui->setStartTimeButton->setText("Edit Start Time");
    ui->setStopTimeButton->setText("Edit Stop Time");
    ui->eventTypeLabel->setText("Type of Event");
    ui->saveButton->setText("Save");
    ui->cancelButton->setText("Cancel");
    ui->calendarWidget->setLocale(QLocale::English);
    break;

    case ELanguage::Default:
    break;
  }
}

void BookingEditor::instance(QWidget* parent)
{
  std::unique_ptr<BookingEditor> bookingEditor = std::make_unique<BookingEditor>(parent);
  bookingEditor->exec();
}

void BookingEditor::updateExistingBookingsLabel(const QDate& date)
{
  Backend::updateBookingsOnSelectedDate(date);

  const QVector<Booking>& bosd = Backend::bookingsOnSelectedDate;

  if (bosd.isEmpty() ||
     (bosd.size() == 1 && bosd[0].index == booking.index)) {
        ui->bookingsOnSelectedDateLabel->setText(
          Backend::language != ELanguage::German ?
          "There are no other bookings yet on " + date.toString() + "." :
          "Es gibt keine anderen Buchungen am " + date.toString() + "."
        );

        return;
  }

  QString str = Backend::language != ELanguage::German ?
    "The following times have been booked on " + date.toString() + ":\n" :
    "Am " + date.toString() + " wurden folgende Zeiten gebucht:\n";

  str += "<html><head/><body>";

  for (const Booking& b : bosd) {
    if (b.index == booking.index) continue;

    bool isConflicting = false;
    
    if (Backend::bookingsAreConflicting(booking, b)) 
      isConflicting = true; 
          
    if (isConflicting)
      str += "<span style=\"background-color: rgb(31, 30, 31); color: rgb(254, 253, 254);\">";

    str += b.startTime.toString("HH:mm") + "-" + b.stopTime.toString("HH:mm") + ": " + b.event + " (" + b.email + ")";

    if (isConflicting) {
      str += Backend::language != ELanguage::German ?
        " CONFLICTING!</span>" : 
        " BUCHUNGSKONFLIKT!</span>";
    }

    str += "<br/>";
  }

  str += "</body></html>";

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  ui->bookingsOnSelectedDateLabel->setText(str);
}

void BookingEditor::drawTimespan()
{
  ui->timeLabel->setText(booking.startTime.toString("HH:mm") + " – " + booking.stopTime.toString("HH:mm"));
}


void BookingEditor::on_calendarWidget_clicked(QDate date)
{
  booking.date = date;

  if (!ui->bookingsOnSelectedDateLabel) return;

  updateExistingBookingsLabel(date);
}

void BookingEditor::on_setStartTimeButton_pressed()
{
  ui->setStartTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px;  }");
  ui->setStopTimeButton->setStyleSheet(
    "QPushButton { background-color: rgb(229, 230, 230); font-size: 16px;  }"
    "QPushButton:hover {background-color: #ffbf00; font-size: 16px;}"  
  );

  timeToSet = ETimeToSet::StartTime;
}
  
void BookingEditor::on_setStopTimeButton_pressed()
{
  ui->setStopTimeButton->setStyleSheet("QPushButton { background-color: #ffbf00; font-size: 16px;  }");
  ui->setStartTimeButton->setStyleSheet(
    "QPushButton { background-color: rgb(229, 230, 230); font-size: 16px;  }"
    "QPushButton:hover {background-color: #ffbf00; font-size: 16px;}"  
  );

  timeToSet = ETimeToSet::StopTime;
}

void BookingEditor::on_decreaseTimeBy60Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking.startTime.addSecs(60 * -60);

    if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    booking.startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking.stopTime.addSecs(60 * -60);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    if (newStopTime <= booking.startTime) {
      QTime newStartTime = newStopTime.addSecs(60 * -60);
      booking.startTime = newStartTime > QTime(7, 0) ? newStartTime : QTime(7, 0);
    }

    booking.stopTime = newStopTime;
  }

  drawTimespan();
}

void BookingEditor::on_decreaseTimeBy05Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking.startTime.addSecs(60 * -5);

   if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    booking.startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking.stopTime.addSecs(60 * -5);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    if (newStopTime <= booking.startTime) {
      QTime newStartTime = newStopTime.addSecs(60 * -5);
      booking.startTime = newStartTime > QTime(7, 0) ? newStartTime : QTime(7, 0);
    }

    booking.stopTime = newStopTime;
  }

  drawTimespan();
}

void BookingEditor::on_increaseTimeBy05Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking.startTime.addSecs(60 * 5);

    if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    if (newStartTime >= booking.stopTime) {
      QTime newStopTime = newStartTime.addSecs(60 * 5);
      booking.stopTime = newStopTime < QTime(23, 0) ? newStopTime : QTime(23, 0);
    }

    booking.startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking.stopTime.addSecs(60 * 5);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    booking.stopTime = newStopTime;
  }

  drawTimespan();
}

void BookingEditor::on_increaseTimeBy60Button_pressed()
{
  if (timeToSet == ETimeToSet::StartTime) {
    QTime newStartTime = booking.startTime.addSecs(60 * 60);

    if (newStartTime > QTime(23, 0) || newStartTime < QTime(7, 0))
      return;

    if (newStartTime >= booking.stopTime) {
      QTime newStopTime = newStartTime.addSecs(60 * 60);
      booking.stopTime = newStopTime < QTime(23, 0) ? newStopTime : QTime(23, 0);
    }

    booking.startTime = newStartTime;
  }

  if (timeToSet == ETimeToSet::StopTime) {
    QTime newStopTime = booking.stopTime.addSecs(60 * 60);

    if (newStopTime > QTime(23, 0) || newStopTime < QTime(7, 0))
      return;

    booking.stopTime = newStopTime;
  }

  drawTimespan();
}

void BookingEditor::on_eventTypeLineEdit_textChanged(const QString& text)
{
  booking.event = text;
}

void BookingEditor::on_saveButton_clicked()
{
  if (booking.event == "") {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please specify the type of event!" :
      "Bitte geben Sie die Art der Veranstaltung an"
    );

    return;
  }

  if (booking.startTime.toString("HH:mm") == booking.stopTime.toString("HH:mm")) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Start and stop time are identical. Please select a reasonable time frame!" :
      "Start- und Stopzeit sind identisch. Bitte wählen Sie einen geeigneten Zeitraum!"
    );

    return;
  }
  
  Backend::updateConflictingBookings(booking.date);

  if (isEditing) 
    JsonParser::updateBooking(booking);
  
  else 
    JsonParser::addBooking(booking);

  Widgets::bookingManager->loadBookings();

  hide();
}

void BookingEditor::on_cancelButton_clicked()
{
  hide();
}