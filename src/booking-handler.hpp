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

  static Booking currentBooking;
  static QVector<Booking> loadedBookings;
  static QVector<Booking> bookingsOnSelectedDate;

  static void updateConflictingBookings(const QDate& date, const bool setConflicting = true);
  static void updateConflictingBookings(Booking& booking, const bool setConflicting = true);
  static bool bookingsAreConflicting(const Booking& booking0, const Booking& booking1);
  static void updateBookingsOnSelectedDate(const QDate& date);
  static void loadBookings();
  static void sortBookings();
  static void reevaluateConflicts();
  static void roundTime(QTime& time);
  static QString makeEntry(const Booking& booking);
};