#include <QLabel>

#include "backend.hpp"
#include "widgets.hpp"
#include "json-parser.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "ui_booking-editor.h"
#include "booking-editor.hpp"

BookingEditor::BookingEditor(QWidget* parent)
  : QDialog(parent),
    ui(new Ui::BookingEditor),
    booking(Backend::currentBooking)
{
  ui->setupUi(this);
  
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
    
    ui->calendarWidget->setSelectedDate(bookingToEdit->date);
    updateExistingBookingsLabel(bookingToEdit->date);

    ui->startTimeEdit->setTime(bookingToEdit->startTime);
    ui->stopTimeEdit->setTime(bookingToEdit->stopTime);
    ui->eventTypeLineEdit->setText(bookingToEdit->event);

    booking.email = bookingToEdit->email;
    booking.index = bookingToEdit->index;
  }

  booking.date = ui->calendarWidget->selectedDate();
  booking.startTime = ui->startTimeEdit->time();
  booking.stopTime = ui->stopTimeEdit->time();
  booking.event = ui->eventTypeLineEdit->text();

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);

  updateExistingBookingsLabel(booking.date);

  exec();
  raise();
}

void BookingEditor::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    ui->bookingsOnSelectedDateLabel->setText("Kein Datum ausgewählt");
    ui->startTimeLabel->setText("Startzeit");
    ui->stopTimeLabel->setText("Stopzeit");
    ui->eventTypeLabel->setText("Art der Veranstaltung");
    ui->saveButton->setText("Speichern");
    ui->cancelButton->setText("Verwerfen");
    ui->calendarWidget->setLocale(QLocale::German);
    break;

    case ELanguage::English:
    ui->bookingsOnSelectedDateLabel->setText("No date selected.");
    ui->startTimeLabel->setText("Start Time");
    ui->stopTimeLabel->setText("Stop Time");
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

  const Booking& booking = Backend::currentBooking;
  const QVector<Booking>& bosd = Backend::bookingsOnSelectedDate;

  if (bosd.isEmpty() ||
     (bosd.size() == 1 && bosd[0].index == booking.index)) {
        ui->bookingsOnSelectedDateLabel->setText(
          Backend::language != ELanguage::German ?
          "There are no bookings yet on " + date.toString() + "." :
          "Es gibt noch keine Buchungen am " + date.toString() + "."
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
      str += "<span style=\"background-color:red;\">";

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

void BookingEditor::on_calendarWidget_clicked(QDate date)
{
  booking.date = date;

  if (!ui->bookingsOnSelectedDateLabel) return;

  updateExistingBookingsLabel(date);
}

void BookingEditor::on_startTimeEdit_timeChanged(QTime time)
{
  booking.startTime = time;

  if (time > booking.stopTime) 
    ui->stopTimeEdit->setTime(time);

  updateExistingBookingsLabel(booking.date);
}

void BookingEditor::on_stopTimeEdit_timeChanged(QTime time)
{
  booking.stopTime = time;

  if (time < booking.startTime) 
    ui->startTimeEdit->setTime(time);

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
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ?
      "Please specify the type of event!" :
      "Bitte geben Sie die Art der Veranstaltung an"
    );

    return;
  }

  if (ui->startTimeEdit->time() == ui->stopTimeEdit->time()) {
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

  bookingManager->loadBookings();

  hide();
}

void BookingEditor::on_cancelButton_pressed()
{
  hide();
}