#pragma once

#include "booking.h"

class JsonParser
{
public:

  static void addBooking(const Booking& booking);
  static void updateBooking(const Booking& booking);
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

  inline static void setBookingsPath(const QString& newPath) { BOOKINGS_PATH = newPath + "bookings.json"; }
  inline static void setPresetsPath(const QString& newPath) { BOOKINGS_PATH = newPath + "bookings.json"; }

private:

  static QString BOOKINGS_PATH;
  static QString PRESETS_PATH;
};