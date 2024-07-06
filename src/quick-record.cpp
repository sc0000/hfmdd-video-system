#include <QLabel>
#include <QScreen>

#include "backend.hpp"
#include "widgets.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "settings-manager.hpp"
#include "booking-manager.hpp"
#include "mode-select.hpp"
#include "ui_quick-record.h"
#include "quick-record.hpp"

QuickRecord* QuickRecord::instance = nullptr;

QuickRecord::QuickRecord(QWidget* parent)
  : FullScreenDialog(parent), 
    ui(new Ui::QuickRecord),
    booking(Backend::currentBooking)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  
  setWindowTitle("Booking Manager");
  instance = this;
  ui->setupUi(this);
  center(ui->masterWidget);

  setModal(false);
  hide();
  
  Backend::loadBookings();
}

QuickRecord::~QuickRecord()
{
  delete ui;
}

void QuickRecord::reload()
{
  raise();
  center(ui->masterWidget);

  Backend::reevaluateConflicts();

  Backend::loadBookings();
  booking.date = QDate::currentDate();
  booking.startTime = QTime::currentTime();
  booking.stopTime = booking.startTime.addSecs(60 * 60);
  booking.email = Backend::currentEmail;
  booking.index = JsonParser::availableIndex();
  booking.event = "Quick Record";

  Backend::roundTime(booking.stopTime);
  Backend::updateBookingsOnSelectedDate(booking.date);
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}

void QuickRecord::translate(ELanguage language)
{
  switch (language) {
    case ELanguage::German:
    ui->recordUntilLabel->setText("Aufnehmen bis...");
    ui->toPTZControlsButton->setText("Zur Kamerasteuerung");
    ui->toModeSelectButton->setText("Zurück");
    break;

    case ELanguage::English:
    ui->recordUntilLabel->setText("Record until...");
    ui->toPTZControlsButton->setText("Go to Camera Controls");
    ui->toModeSelectButton->setText("Go back to Mode Selection");
    break;

    case ELanguage::Default:
    break;
  }
}

void QuickRecord::updateExistingBookingsLabel(QDate date)
{
  Backend::updateBookingsOnSelectedDate(date);

  const QVector<Booking>& bosd = Backend::bookingsOnSelectedDate;

  if (bosd.isEmpty() ||
     (bosd.size() == 1 && bosd[0].index == booking.index)) {
        ui->bookingsOnSelectedDateLabel->setText(Backend::language != ELanguage::German ? 
          "There are no bookings yet on " + date.toString() + "." : 
          "Es gibt noch keine Buchungen am " + date.toString() + ".");
        return;
  }

  QString str = Backend::language != ELanguage::German ? 
    "The following times have been booked later today:\n" :
    "Heute sind bereits folgende Zeiten gebucht:\n";

  str += "<html><head/><body>";

  for (const Booking& b : bosd) {
    if (b.index == booking.index ||
        b.stopTime < booking.startTime) continue;

    bool isConflicting = false;
    
    if (Backend::bookingsAreConflicting(booking, b)) 
      isConflicting = true; 
          
    if (isConflicting)
      str += "<span style=\"background-color: rgb(31, 30, 31); color: rgb(254, 253, 254);\">";

    str += b.startTime.toString("HH:mm") + "-" + 
      b.stopTime.toString("HH:mm") + ": " + 
      b.event + " (" + b.email + ")";

    if (isConflicting)
      str += " CONFLICTING!</span>";

    str += "<br/>";
  }

  str += "</body></html>";

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  ui->bookingsOnSelectedDateLabel->setText(str);
}

void QuickRecord::updateConflictingBookings(const QDate& date)
{

}

void QuickRecord::updateStopTimeLabel()
{ 
  ui->stopTimeLabel->setText(booking.stopTime.toString("HH:mm"));
}

void QuickRecord::on_decreaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -20);

  if (newStopTime <= booking.startTime) {
    Widgets::okDialog->display("Invalid time");
    return;
  }

  if (newStopTime > QTime(23, 0)) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ? 
      "Recordings can't extend beyond 23:00" :
      "Es kann nicht über 23 Uhr hinaus aufgenommen werden"
    );

    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}

void QuickRecord::updateDurationLabel()
{
  int secs = QTime::currentTime().secsTo(booking.stopTime);
  int totalMin = secs / 60;
  QString hours = QString::number(totalMin / 60);
  QString minutes = QString::number(totalMin % 60);

  ui->durationLabel->setText(Backend::language != ELanguage::German ?
    "( " + hours + "h " + minutes + "min from now )" : 
    "( " + hours + "h " + minutes + "min ab jetzt )" 
  );
}

void QuickRecord::on_decreaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -5);

  if (newStopTime <= booking.startTime) {
    Widgets::okDialog->display("Invalid time");
    return;
  }

  if (newStopTime > QTime(23, 0)) {
    Widgets::okDialog->display(
      Backend::language != ELanguage::German ? 
      "Recordings can't extend beyond 23:00" :
      "Es kann nicht über 23 Uhr hinaus aufgenommen werden"
    );
    
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}

void QuickRecord::on_increaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * 5);

  if (newStopTime > QTime(23, 0)) {
    Widgets::okDialog->display("Recording can't extend beyond 23:00");
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}

void QuickRecord::on_increaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * 20);

  if (newStopTime > QTime(23, 0)) {
    Widgets::okDialog->display("Recordings can't extend beyond 23:00");
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}


void QuickRecord::on_toPTZControlsButton_clicked()
{ 
  Backend::updateConflictingBookings(booking.date);
  JsonParser::addBooking(booking);

  // TODO: Check: always reset or update, and setup option to reset manually?
  SettingsManager::resetFilterSettings();

  Widgets::ptzControls->reload();
  Widgets::showFullScreenDialogs(false);
}

void QuickRecord::on_toModeSelectButton_clicked()
{
  fade(Widgets::modeSelect);
}

void QuickRecord::on_logoutButton_clicked()
{
  fade(Widgets::loginDialog);
}