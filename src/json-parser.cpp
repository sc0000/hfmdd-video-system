#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "json-parser.hpp"

#include <QVector>
#include <QDate>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// QString JsonParser::BOOKINGS_PATH = "V:/sb-terminal-test/bookings.json";
QString JsonParser::BOOKINGS_PATH = "C:/Users/sebas/OneDrive/Dokumente/OBS-RecordingsTEST/bookings.json";

void JsonParser::addBooking(const Booking& booking)
{
  QJsonArray arr = 
    readJsonArrayFromFile(BOOKINGS_PATH);

  QJsonObject bookingObj;
  bookingObj.insert("Email", booking.email);
  bookingObj.insert("Date", booking.date.toString(Qt::ISODate));
  bookingObj.insert("StartTime", booking.startTime.toString());
  bookingObj.insert("StopTime", booking.stopTime.toString());
  bookingObj.insert("Event", booking.event);

  arr.append(bookingObj);
  writeJsonArrayToFile(arr, BOOKINGS_PATH);
}

void JsonParser::updateBooking(qsizetype index, const Booking& booking)
{
  QJsonArray arr = 
    readJsonArrayFromFile(BOOKINGS_PATH);

  QJsonObject bookingObj;
  bookingObj.insert("Email", booking.email);
  bookingObj.insert("Date", booking.date.toString(Qt::ISODate));
  bookingObj.insert("StartTime", booking.startTime.toString());
  bookingObj.insert("StopTime", booking.stopTime.toString());
  bookingObj.insert("Event", booking.event);

  arr[index] = bookingObj;
  writeJsonArrayToFile(arr, BOOKINGS_PATH);
}

void JsonParser::getBookingsOnDate(const QDate& date, QVector<Booking>& outVector)
{
  outVector.clear();

  QJsonArray arr = readJsonArrayFromFile(BOOKINGS_PATH);

  for (const QJsonValue& val : arr)
  {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if (!obj.contains("Date") || obj.value("Date").toString() != date.toString(Qt::ISODate))
      continue;

    Booking booking =
      {
        obj.value("Email").toString(),
        QDate::fromString(obj.value("Date").toString()),
        QTime::fromString(obj.value("StartTime").toString()),
        QTime::fromString(obj.value("StopTime").toString()),
        obj.value("Event").toString()
      };

      outVector.append(booking);
  }
}

void JsonParser::getBookingsForEmail(const QString& mailAddress, QVector<Booking>& outVector)
{
  outVector.clear();

  QJsonArray arr = readJsonArrayFromFile(BOOKINGS_PATH);

  if (arr.isEmpty()) return;

  for (const QJsonValue& val : arr)
  {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if (!obj.contains("Email") || obj.value("Email").toString() != mailAddress)
      continue;

    Booking booking =
      {
        obj.value("Email").toString(),
        QDate::fromString(obj.value("Date").toString(), Qt::ISODate),
        QTime::fromString(obj.value("StartTime").toString("HH:mm")),
        QTime::fromString(obj.value("StopTime").toString("HH:mm")),
        obj.value("Event").toString()
      };

      outVector.append(booking);
  }
}

void JsonParser::removeBooking(const Booking& booking)
{
  QJsonArray arr = readJsonArrayFromFile(BOOKINGS_PATH);

  for (qsizetype i = arr.size() - 1; i >= 0; --i) {
    QJsonValue val = arr[i];

    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if (obj.value("Email").toString() == booking.email &&
        obj.value("Date").toString() == booking.date.toString(Qt::ISODate) &&
        obj.value("StartTime").toString() == booking.startTime.toString() &&
        obj.value("StopTime").toString() == booking.stopTime.toString())
        arr.removeAt(i);
  }

  writeJsonArrayToFile(arr, BOOKINGS_PATH); 
}

QJsonArray JsonParser::readJsonArrayFromFile(const QString& path)
{
  QFile file(path);
  
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return QJsonArray();

  QString jsonStr = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());

  if (!doc.isArray()) 
    return QJsonArray();
  
  return doc.array();
}

void JsonParser::writeJsonArrayToFile(const QJsonArray& arr, const QString& path)
{
  QJsonDocument doc;
  doc.setArray(arr);
  
  QFile file(path);
  
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) 
    return;

  file.write(doc.toJson());
  file.close();
}