#include <QLabel>
#include <QScreen>

#include "backend.hpp"
#include "login.hpp"
#include "login-dialog.hpp"
#include "message-dialog.hpp"
#include "booking-editor.hpp"
#include "json-parser.hpp"
#include "ptz-controls.hpp"
#include "path-manager.hpp"
#include "booking-manager.hpp"
#include "globals.hpp"
#include "mode-select.hpp"
#include "ui_quick-record.h"
#include "quick-record.hpp"


QuickRecord* QuickRecord::instance = nullptr;

QuickRecord::QuickRecord(QWidget* parent)
  : QDialog(parent), 
    ui(new Ui::QuickRecord)
{
  setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::FramelessWindowHint);
  
  setWindowTitle("Booking Manager");
  instance = this;
  ui->setupUi(this);
  repositionMasterWidget();

  showFullScreen();
  
  loadBookings();

  booking.date = QDate::currentDate();
  booking.startTime = QTime::currentTime();
  booking.stopTime = booking.startTime.addSecs(60 * 60);
  booking.email = Backend::currentEmail;
  booking.index = JsonParser::availableIndex();
  booking.event = "Quick Record";

  roundTime(booking.stopTime);
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
}

QuickRecord::~QuickRecord()
{
  delete ui;
}

void QuickRecord::reload()
{
  repositionMasterWidget();
  show();
}

void QuickRecord::repositionMasterWidget()
{
  Globals::centerFullScreenWidget(ui->masterWidget);
}

void QuickRecord::loadBookings()
{   
  QVector<Booking> tmp;

  if (Backend::currentEmail == Backend::adminEmail)
    JsonParser::getAllBookings(tmp);

  else   
    JsonParser::getBookingsForEmail(Backend::currentEmail, tmp);

  bookings.clear();  

  for (const Booking& b : tmp) {
   bookings.append(b);
  }

  sortBookings();

  // if (!ui->bookingsList) return;

  // ui->bookingsList->clear();

  // for (qsizetype i = 0; i < bookings.size(); ++i) {
  //   QString entryText = makeEntry(bookings[i]);
  //   QListWidgetItem* item = new QListWidgetItem(entryText);
    
  //   if (bookings[i].isConflicting)
  //     item->setBackground(Qt::red);

  //   ui->bookingsList->addItem(item);
  // }
}

void QuickRecord::sortBookings()
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

QString QuickRecord::makeEntry(const Booking& booking)
{
  QString entry = 
      booking.date.toString("ddd MMM dd yyyy") + "\t" +
      booking.startTime.toString("HH:mm") + " - " +
      booking.stopTime.toString("HH:mm") + "\t" +
      booking.event.leftJustified(20, ' ') + "\t" +
      booking.email.leftJustified(20, ' ') +
      (booking.isConflicting ? "\t--CONFLICTING!" : "");

  return entry;
}

void QuickRecord::updateExistingBookingsLabel(QDate date)
{
  bookingsOnSelectedDate.clear();
  
  JsonParser::getBookingsOnDate(date, bookingsOnSelectedDate);

  if (bookingsOnSelectedDate.isEmpty() ||
     (bookingsOnSelectedDate.size() == 1 && bookingsOnSelectedDate[0].index == booking.index)) {
        ui->bookingsOnSelectedDateLabel->setText("There are no bookings yet on " + date.toString() + ".");
        return;
  }

  QString str = "The following times have been booked later today:\n ";

  str += "<html><head/><body>";

  for (Booking& b : bookingsOnSelectedDate) {
    if (b.index == booking.index ||
        b.stopTime < booking.startTime) continue;

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

  ui->bookingsOnSelectedDateLabel->setTextFormat(Qt::RichText);
  ui->bookingsOnSelectedDateLabel->setText(str);
}

void QuickRecord::updateConflictingBookings(const QDate& date)
{

}

bool QuickRecord::bookingsAreConflicting(const Booking& booking0, const Booking& booking1)
{
  if (booking0.index == booking1.index) return false;

  if ((booking0.startTime >= booking1.startTime && booking0.startTime < booking1.stopTime) ||
      (booking0.stopTime > booking1.startTime && booking0.stopTime <= booking1.stopTime))
    return true;

  return false; 
}

void QuickRecord::roundTime(QTime& time)
{
  int minutes = time.minute();
  int remainder = minutes % 10;
  int roundedMinutes = minutes - remainder;

  if (remainder > 4)
    roundedMinutes += 10;

  if (roundedMinutes >= 60) {
    roundedMinutes = 0;
    int hour = time.hour();

    if (hour == 23) 
      time = QTime(0, 0);

    else 
      time = QTime(hour + 1, 0);
  }

  else
    time = QTime(time.hour(), roundedMinutes);
}

void QuickRecord::updateStopTimeLabel()
{ 
  ui->stopTimeLabel->setText(booking.stopTime.toString("HH:mm"));
}

void QuickRecord::on_decreaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -20);

  if (newStopTime <= booking.startTime) {
    OkDialog::instance("Invalid time", this);
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
}

void QuickRecord::on_decreaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * -5);

  if (newStopTime <= booking.startTime) {
    OkDialog::instance("Invalid time", this);
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
}

void QuickRecord::on_increaseTimeBy05Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * 5);

  if (newStopTime > QTime(23, 0)) {
    OkDialog::instance("Recording can't extend beyond 23:00", this);
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
}

void QuickRecord::on_increaseTimeBy20Button_pressed()
{
  QTime newStopTime = booking.stopTime.addSecs(60 * 20);

  if (newStopTime > QTime(23, 0)) {
    OkDialog::instance("Recordings can't extend beyond 23:00", this);
    return;
  }

  booking.stopTime = newStopTime;
  updateStopTimeLabel();
  updateExistingBookingsLabel(booking.date);
}


void QuickRecord::on_toPTZControlsButton_pressed()
{
  PathManager::outerDirectory = booking.date.toString(Qt::ISODate) + "/";
  
  // TODO: Check: always reset or update, and setup option to reset manually?
  PathManager::resetFilterSettings();

  hide();

  Login::getInstance()->hide();
  LoginDialog::getInstance()->hide();

  PTZControls* ptzControls = PTZControls::getInstance();

  if (!ptzControls) return;

  ptzControls->prepare();
}

void QuickRecord::on_toModeSelectButton_pressed()
{
  ModeSelect* modeSelect = ModeSelect::getInstance();

  if (!modeSelect) return;

  modeSelect->reload();
  hide();
}

void QuickRecord::on_logoutButton_pressed()
{
  LoginDialog* loginDialog = LoginDialog::getInstance();
  
  if (!loginDialog) return;

  loginDialog->reload();
  hide();
}