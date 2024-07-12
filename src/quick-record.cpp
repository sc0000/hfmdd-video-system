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

  ui->infoButton->setStyleSheet(
    "QPushButton { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
    "QPushButton:hover { background-color: rgb(42, 130, 218); }"
    "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
  );

  ui->infoLabel->setStyleSheet(
    "QLabel { background-color: rgb(42, 130, 218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
  );

  ui->infoLabel->setText(
    QString("<html><head/><body>") + 
      "<span style=\"font-weight: bold;\">What am I looking at?</span><br/>" +
      "From this screen, you can just set a stop time and move on to record a video right away. " +
      "Just take other booked recordings happening today into account.<br/><br/>" +
      "<span style=\"font-weight: bold;\">Why do I even have to set a stop time?</span><br/>" +
      "You can stop the recording whenever you want, but " +
      "<span style=\"font-weight: bold;\">(IMPORTANT) </span>" +
      "the recording will stop automatically 10 minutes after the designated time!<br/><br/>" +
      "<span style=\"font-weight: bold;\">And then what?</span><br/>" +
      "No matter how the recording was stopped, you will receive a download link via email afterwards." +
      "</body></html>"
  );

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
  if (!Backend::mailAddressIsValid) {
    Widgets::loginDialog->reload();
    return;
  }

  ui->infoLabel->hide();

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

    ui->infoLabel->setFont(QFont("DaxOT", 11));
    ui->infoLabel->setText(
      QString("<html><head/><body>") + 
        "Von diesem Bildschirm können Sie direkt weiter zur Videoaufnahme.<br/><br/>" +
        "--Dafür müssen Sie lediglich die Zeit bestimmen, zu der die Aufnahme voraussichtlich endet. " +
        "Bitte berücksichtigen Sie dabei weitere für heute gebuchte Aufnahmen.<br/><br/>" +
        "--Sie können die Aufnahme jederzeit stoppen, aber:<br/><br/>" +
        "<span style=\"font-weight: bold;\">" +
        "--10 Minuten nach der angegeben Zeit wird die Aufnahme automatisch gestoppt!</span><br/><br/>" +
        "--Nach Ende der Aufnahme (egal ob manuell oder automatisch ausgelöst) erhalten Sie einen Downloadlink per E-Mail." +
        "</body></html>"
    );
    break;

    case ELanguage::English:
    ui->recordUntilLabel->setText("Record until...");
    ui->toPTZControlsButton->setText("Go to Camera Controls");
    ui->toModeSelectButton->setText("Go back to Mode Selection");

    ui->infoLabel->setFont(QFont("DaxOT", 12));
    ui->infoLabel->setText(
      QString("<html><head/><body>") + 
        "From this screen, you can move on to record a video right away.<br/><br/>" +
        "--Just set the time at which you estimate the recording to end. " +
        "Please take other booked recordings happening today into account.<br/><br/>" +
        "--You can stop the recording whenever you want, but:<br/><br/>" +
        "<span style=\"font-weight: bold;\">" +
        "--The recording will stop automatically 10 minutes after the designated time!</span><br/><br/>" +
        "--No matter how the recording was stopped, you will receive a download link via email afterwards." +
        "</body></html>"
    );

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
          "There are no bookings yet today." : 
          "Es gibt noch keine Buchungen am heutigen Tag.");
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

void QuickRecord::on_infoButton_pressed()
{
  if (ui->infoLabel->isHidden()) {
    ui->infoButton->setStyleSheet(
      "QPushButton { background-color: rgb(42, 130, 218); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(42, 130, 218); }"
      "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
    );

    ui->infoLabel->show();
  }
    
  else {
    ui->infoButton->setStyleSheet(
      "QPushButton { background-color: rgb(31, 30, 31); color: rgb(254, 253, 254); border: 1px solid rgb(254, 253, 254); }"
      "QPushButton:hover { background-color: rgb(42, 130, 218); }"
      "QPushButton:pressed { background-color: rgb(42, 130, 218); }"
    );

    ui->infoLabel->hide();
  }
}

void QuickRecord::on_decreaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -20);

  if (newStopTime <= booking.startTime) {
    // Widgets::okDialog->display("Invalid time");
    return;
  }

  if (newStopTime > QTime(23, 0)) {
    // Widgets::okDialog->display(
    //   Backend::language != ELanguage::German ? 
    //   "Recordings can't extend beyond 23:00" :
    //   "Es kann nicht über 23 Uhr hinaus aufgenommen werden"
    // );

    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
  updateDurationLabel();
}

void QuickRecord::on_decreaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -5);

  if (newStopTime <= booking.startTime) {
    // Widgets::okDialog->display("Invalid time");
    return;
  }

  if (newStopTime > QTime(23, 0)) {
    // Widgets::okDialog->display(
    //   Backend::language != ELanguage::German ? 
    //   "Recordings can't extend beyond 23:00" :
    //   "Es kann nicht über 23 Uhr hinaus aufgenommen werden"
    // );
    
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
    // Widgets::okDialog->display("Recording can't extend beyond 23:00");
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
   //  Widgets::okDialog->display("Recordings can't extend beyond 23:00");
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