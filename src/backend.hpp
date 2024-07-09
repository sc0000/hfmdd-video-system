#pragma once

#include <QVector>

#include "language.hpp"
#include "booking.h"

enum class EMode
{
  QuickMode,
  BookMode,
  Default
};

enum class EMailType
{
  SendFiles,
  BookingConflictWarning,
  Default
};

class Backend
{
public:
  static QString currentEmail;
  static bool mailAddressIsValid;
  const static QString adminEmail;
  const static QString adminPassword;
  static EMode mode;
  static ELanguage language;

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

  static QString sendFiles(const Booking& booking);
  static QString sendMail(const Booking& booking, EMailType mailType);
};