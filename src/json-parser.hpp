#pragma once

#include "booking.h"

class JsonParser
{
public:

  static void addBooking(const Booking& booking);
  static void updateBooking(const Booking& booking);
  static void updateAllBookings(const QVector<Booking>& allBookings);
  static void getBookingsForEmail(const QString& mailAddress, QVector<Booking>& outVector);
  static void getBookingsOnDate(const QDate& date, QVector<Booking>& outVector);
  static void getAllBookings(QVector<Booking>& outVector);
  static void removeBooking(const Booking& booking);
  static int availableIndex();

  static void addPreset(const QString& email, const int preset);
  static void removePreset(const QString& email, const int preset);
  static void getPresetsForEmail(const QString& email, QVector<int>& outVector);

  static QJsonArray readJsonArrayFromFile(const QString& path);
  static void writeJsonArrayToFile(const QJsonArray& arr, const QString& path);

  inline static void setBookingsPath(const QString& newPath) { bookingsPath = newPath + "bookings.json"; }
  inline static void setPresetsPath(const QString& newPath) { presetsPath = newPath + "bookings.json"; }

  static QString bookingsPath;
  static QString presetsPath;
};