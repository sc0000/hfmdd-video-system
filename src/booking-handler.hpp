#pragma once

#include <QVector>
#include <QString>
#include <QDateTime>

enum class EMode
{
  QuickMode,
  BookMode,
  Default
};

struct Booking
{
  QString email;
  QDate date;
  QTime startTime;
  QTime stopTime;
  QString event;
  bool isConflicting;
  int index;
};

class BookingHandler
{
public:
  static EMode mode;

  static QDate selectedDate;

  static Booking* currentBooking;
  static QVector<Booking> allBookings;
  static QVector<Booking*> loadedBookings;
  static QVector<Booking*> bookingsOnSelectedDate;

  static void initCurrentBooking();
  static void addCurrentBooking();
  static bool bookingsAreConflicting(const Booking* booking0, const Booking* booking1);
  static void getBookingsOnSelectedDate(const QDate& date);
  static void getUserBookings();
  static void saveBookings();
  static void loadBookings();
  static void sortAllBookings();
  static void sortBookings(QVector<Booking*>& bookings);
  static void reevaluateConflicts();
  static void roundTime(QTime& time);

};