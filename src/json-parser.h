#pragma once

#include <QDate>
#include <QVector>

#include "booking.h"

class JsonParser
{
public:

  static QVector<Booking> getBookingsAtDate(const QDate& date);
  static void addBookingToDate(const QDate& date, const Booking& booking);

  static QJsonArray readJsonArrayFromFile(const QString& path);
  static void writeJsonArrayToFile(const QJsonArray& arr, const QString& path);

  inline static void setBookingsPath(const QString& newPath) { BOOKINGS_PATH = newPath + "bookings.json"; }

private:

  static QString BOOKINGS_PATH;

  const static QString DATE_FIELD;
  const static QString BOOKINGS_FIELD;
};