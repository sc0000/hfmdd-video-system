#include "json-parser.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

// #define BOOKINGS_PATH "C:/Users/sebas/OneDrive/Dokumente/OBS-RecordingsTEST/Bookings/bookings.json"

QString JsonParser::BOOKINGS_PATH = "V:/sb-terminal-test/bookings.json";
const QString JsonParser::DATE_FIELD = "00-Date";
const QString JsonParser::BOOKINGS_FIELD = "01-Bookings";

QVector<Booking> JsonParser::getBookingsAtDate(const QDate& date)
{
  QVector<Booking> outVector;

  QJsonArray arr = 
    readJsonArrayFromFile(BOOKINGS_PATH);

  for (const QJsonValue& val : arr)
  {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if (obj.value(DATE_FIELD) != date.toString(Qt::ISODate))
      continue;

    QJsonArray bookings = obj.value(BOOKINGS_FIELD).toArray();

    for (const QJsonValue& bookingVal : bookings)
    {
      if (!bookingVal.isObject()) continue;

      QJsonObject bookingObj = bookingVal.toObject();

      Booking booking =
      {
        bookingObj.value("Email").toString(),
        QTime::fromString(bookingObj.value("StartTime").toString()),
        QTime::fromString(bookingObj.value("StopTime").toString())
      };

      outVector.append(booking);
    }
  }
  
  return outVector;
}

void JsonParser::addBookingToDate(const QDate& date, const Booking& booking)
{
  QJsonArray arr = 
      readJsonArrayFromFile(BOOKINGS_PATH);

  QJsonObject bookingObj;
  bookingObj.insert("Email", booking.email);
  bookingObj.insert("StartTime", booking.startTime.toString());
  bookingObj.insert("StopTime", booking.stopTime.toString());

  for (qsizetype i = 0; i < arr.size(); ++i) // for (const QJsonValue& val : arr)
  {
    if (!arr[i].isObject()) continue;

    QJsonObject obj = arr[i].toObject();

    if (obj.value(DATE_FIELD) == date.toString(Qt::ISODate))
    {
      QJsonArray bookingsArr = obj.value(BOOKINGS_FIELD).toArray();
      bookingsArr.append(bookingObj);

      obj[BOOKINGS_FIELD] = bookingsArr;
      arr[i] = obj;

      writeJsonArrayToFile(arr, BOOKINGS_PATH);

      return;
    } 
  }

  QJsonArray bookingsArr;
  bookingsArr.append(bookingObj);

  QJsonObject dateObj;
  dateObj.insert("00-Date", date.toString(Qt::ISODate));
  dateObj.insert("01-Bookings", bookingsArr);

  arr.append(dateObj);
  
  writeJsonArrayToFile(arr, BOOKINGS_PATH);
}

QJsonArray JsonParser::readJsonArrayFromFile(const QString& path)
{
  QVector<Booking> outVector;

  QFile file(path);
  file.open(QIODevice::ReadOnly | QIODevice::Text);

  QString jsonStr = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
  QJsonArray arr = doc.array();

  return arr;
}

void JsonParser::writeJsonArrayToFile(const QJsonArray& arr, const QString& path)
{
  QJsonDocument doc;
  doc.setArray(arr);
  
  QFile file(path);
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  file.write(doc.toJson());
  file.close();
}