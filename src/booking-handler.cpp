#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

#include "json-parser.hpp"
#include "widgets.hpp"
#include "message-dialog.hpp"
#include "mail-handler.hpp"
#include "text-handler.hpp"
#include "booking-handler.hpp"

EMode BookingHandler::mode = EMode::Default;

QDate BookingHandler::selectedDate = QDate();

Booking* BookingHandler::currentBooking = nullptr;
QVector<Booking> BookingHandler::allBookings = {};
QVector<Booking*> BookingHandler::loadedBookings = {};
QVector<Booking*> BookingHandler::bookingsOnSelectedDate = {};

void BookingHandler::initCurrentBooking()
{
  currentBooking = new Booking;
}

void BookingHandler::addCurrentBooking()
{
  for (const Booking& booking : allBookings) {
    if (currentBooking->index == booking.index)
      return;
  }

  allBookings.append(*currentBooking);
}

bool BookingHandler::bookingsAreConflicting(const Booking* b0, const Booking* b1)
{
  if (b0->index == b1->index ||
      b0->date != b1->date) return false;

  if ((b0->startTime >= b1->startTime && b0->startTime < b1->stopTime) ||
      (b0->stopTime > b1->startTime && b0->stopTime <= b1->stopTime) ||
      (b0->startTime <= b1->startTime && b0->stopTime >= b1->stopTime))
    return true;

  return false; 
}

void BookingHandler::getBookingsOnSelectedDate(const QDate& date)
{
  bookingsOnSelectedDate.clear();

  for (Booking& booking : allBookings) {
    if (booking.date == date)
      bookingsOnSelectedDate.append(&booking);
  }

  sortBookings(bookingsOnSelectedDate);
}

void BookingHandler::getUserBookings()
{ 
  loadedBookings.clear();

  for (Booking& booking : allBookings) {
    if (MailHandler::isAdmin)
      loadedBookings.append(&booking);

    else if (booking.email == MailHandler::currentEmail)
      loadedBookings.append(&booking);
  }

  sortBookings(loadedBookings); 
}

void BookingHandler::saveBookings()
{
  JsonParser::updateAllBookings(allBookings);
}

void BookingHandler::loadBookings()
{
  JsonParser::getAllBookings(allBookings);
  sortAllBookings();
  getUserBookings();
}

void BookingHandler::sortAllBookings()
{
  qsizetype size = allBookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = allBookings[j].date.toString(Qt::ISODate) + "_" +
        allBookings[j].startTime.toString("HH:mm");

      QString dateTime1 = allBookings[j + 1].date.toString(Qt::ISODate) + "_" +
        allBookings[j + 1].startTime.toString("HH:mm");

      if (dateTime0 < dateTime1) {
        Booking tmp = allBookings[j];
        allBookings[j] = allBookings[j + 1];
        allBookings[j + 1] = tmp;

        flag = true;
      }
    }

    if (flag == 0) break;
  }
}

void BookingHandler::sortBookings(QVector<Booking*>& bookings)
{
  qsizetype size = bookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = bookings[j]->date.toString(Qt::ISODate) + "_" +
                          bookings[j]->startTime.toString("HH:mm");

      QString dateTime1 = bookings[j + 1]->date.toString(Qt::ISODate) + "_" +
                          bookings[j + 1]->startTime.toString("HH:mm");

      if (dateTime0 < dateTime1) {
        qSwap(bookings[j], bookings[j + 1]);
        flag = true;
      }
    }

    if (!flag) break;
  }
}

void BookingHandler::reevaluateConflicts()
{
  qsizetype size = allBookings.size();

  if (size == 0) return;

  for (qsizetype i = 0; i < size; ++i) {
    bool isConflicting = false;

    for (qsizetype j = 0; j < size; ++j) {
      if (bookingsAreConflicting(&allBookings[i], &allBookings[j])) 
        isConflicting = true; 
    }

    allBookings[i].isConflicting = isConflicting;
  }

  JsonParser::updateAllBookings(allBookings);
}

void BookingHandler::roundTime(QTime& time)
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