#pragma once

#include "booking.h"

class JsonParser
{
public:

  static void addBooking(const Booking& booking);
  static void updateBooking(qsizetype index, const Booking& booking);
  static void getBookingsForEmail(const QString& mailAddress, QVector<Booking>& outVector);
  static void getBookingsOnDate(const QDate& date, QVector<Booking>& outVector);
  static void removeBooking(const Booking& booking);
  

  static QJsonArray readJsonArrayFromFile(const QString& path);
  static void writeJsonArrayToFile(const QJsonArray& arr, const QString& path);

  inline static void setBookingsPath(const QString& newPath) { BOOKINGS_PATH = newPath + "bookings.json"; }

private:

  static QString BOOKINGS_PATH;

  const static QString DATE_FIELD;
  const static QString BOOKINGS_FIELD;
};