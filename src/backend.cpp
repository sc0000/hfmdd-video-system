#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

#include "json-parser.hpp"
#include "settings-manager.hpp"
#include "widgets.hpp"
#include "message-dialog.hpp"
#include "mail-handler.hpp"
#include "text-manager.hpp"
#include "backend.hpp"


EMode Backend::mode = EMode::Default;
ELanguage Backend::language = ELanguage::English;

QDate Backend::selectedDate = QDate();

Booking Backend::currentBooking = {0};
QVector<Booking> Backend::loadedBookings = {};
QVector<Booking> Backend::bookingsOnSelectedDate = {};

void Backend::updateConflictingBookings(const QDate& date, const bool setConflicting)
{
  updateBookingsOnSelectedDate(date);

  currentBooking.isConflicting = false;

  for (Booking& b : bookingsOnSelectedDate) {
    if (bookingsAreConflicting(currentBooking, b)) {
      currentBooking.isConflicting = true;
      b.isConflicting = setConflicting;
    }

    else {
      b.isConflicting = false;
    }

    b.date = date;

    JsonParser::updateBooking(b);
  }

  JsonParser::updateBooking(currentBooking);
}

void Backend::updateConflictingBookings(Booking& booking, const bool setConflicting)
{
  updateBookingsOnSelectedDate(booking.date);

  booking.isConflicting = false;

  for (Booking& other : bookingsOnSelectedDate) {
    if (bookingsAreConflicting(booking, other)) {
      booking.isConflicting = true;
      other.isConflicting = setConflicting;
    }

    else {
      other.isConflicting = false;
    }

    other.date = booking.date;

    JsonParser::updateBooking(other);
  }

  JsonParser::updateBooking(booking);
}

bool Backend::bookingsAreConflicting(const Booking& b0, const Booking& b1)
{
  if (b0.index == b1.index) return false;

  if ((b0.startTime >= b1.startTime && b0.startTime < b1.stopTime) ||
      (b0.stopTime > b1.startTime && b0.stopTime <= b1.stopTime))
    return true;

  return false; 
}

void Backend::updateBookingsOnSelectedDate(const QDate& date)
{
  JsonParser::getBookingsOnDate(date, bookingsOnSelectedDate);
}

void Backend::loadBookings()
{
  if (MailHandler::currentEmail == MailHandler::adminEmail)
    JsonParser::getAllBookings(loadedBookings);

  else 
    JsonParser::getBookingsForEmail(MailHandler::currentEmail, loadedBookings);

  sortBookings();
}

void Backend::sortBookings()
{
  qsizetype size = loadedBookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = loadedBookings[j].date.toString(Qt::ISODate) + "_" +
        loadedBookings[j].startTime.toString("HH:mm");

      QString dateTime1 = loadedBookings[j + 1].date.toString(Qt::ISODate) + "_" +
        loadedBookings[j + 1].startTime.toString("HH:mm");

      if (dateTime0 < dateTime1) {
        Booking tmp = loadedBookings[j];
        loadedBookings[j] = loadedBookings[j + 1];
        loadedBookings[j + 1] = tmp;

        flag = true;
      }
    }

    if (flag == 0) break;
  }
}

void Backend::reevaluateConflicts()
{
  // load all bookings (locally)
  QVector<Booking> allBookings;
  JsonParser::getAllBookings(allBookings);

  for (qsizetype i = 0; i < allBookings.size(); ++i) {
    bool isConflicting = false;

    for (qsizetype j = 0; j < allBookings.size(); ++j) {
      if (bookingsAreConflicting(allBookings[i], allBookings[j])) 
        isConflicting = true; 
    }

    allBookings[i].isConflicting = isConflicting;
  }
}

void Backend::roundTime(QTime& time)
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

QString Backend::makeEntry(const Booking& booking)
{
  QString entry = 
    booking.date.toString("ddd MMM dd yyyy") + "\t" +
    booking.startTime.toString("HH:mm") + " - " +
    booking.stopTime.toString("HH:mm") + "\t" +
    booking.event.leftJustified(20, ' ') + "\t" +
    booking.email.leftJustified(20, ' ') +
    (booking.isConflicting ? TextManager::getText(ID::CONFLICT) : "");

  return entry;
}