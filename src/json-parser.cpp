/* Reading from and writing to JSON files (bookings and presets)
 * 
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#include <QVector>
#include <QDate>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "mail-handler.hpp"
#include "storage-handler.hpp"
#include "message-dialog.hpp"
#include "booking-manager.hpp"
#include "json-parser.hpp"

QString JsonParser::bookingsPath = "";
QString JsonParser::presetsPath = "";

void JsonParser::addBooking(const Booking* booking)
{
  if (!booking) return;

  QJsonArray arr = 
    readJsonArrayFromFile(bookingsPath);

  QJsonObject bookingObj;
  bookingObj.insert("Email", booking->email);
  bookingObj.insert("Date", booking->date.toString(Qt::ISODate));
  bookingObj.insert("StartTime", booking->startTime.toString());
  bookingObj.insert("StopTime", booking->stopTime.toString());
  bookingObj.insert("Event", booking->event);
  bookingObj.insert("Conflicting", booking->isConflicting ? "true" : "false");
  bookingObj.insert("Index", QString::number(booking->index));

  arr.append(bookingObj);
  writeJsonArrayToFile(arr, bookingsPath);
}

void JsonParser::updateBooking(const Booking* booking)
{
  if (!booking) return;

  QJsonArray arr = 
    readJsonArrayFromFile(bookingsPath);

  QJsonObject bookingObj;
  bookingObj.insert("Email", booking->email);
  bookingObj.insert("Date", booking->date.toString(Qt::ISODate));
  bookingObj.insert("StartTime", booking->startTime.toString("HH:mm"));
  bookingObj.insert("StopTime", booking->stopTime.toString("HH:mm"));
  bookingObj.insert("Event", booking->event);
  bookingObj.insert("Conflicting", booking->isConflicting ? "true" : "false");
  bookingObj.insert("Index", QString::number(booking->index));

  qsizetype size = arr.size();

  for (qsizetype i = 0; i < size; ++i) {
    const QJsonValue& val = arr[i];

    if (!val.isObject())
      continue;

    QJsonObject obj = val.toObject();

    if (obj.value("Index").toVariant().toInt() == booking->index)
      arr[i] = bookingObj;
  }

  writeJsonArrayToFile(arr, bookingsPath);
}

void JsonParser::updateAllBookings(const QVector<Booking>& allBookings)
{
  QJsonArray arr;

  for (const Booking& booking : allBookings) {
    QJsonObject bookingObj;
    bookingObj.insert("Email", booking.email);
    bookingObj.insert("Date", booking.date.toString(Qt::ISODate));
    bookingObj.insert("StartTime", booking.startTime.toString("HH:mm"));
    bookingObj.insert("StopTime", booking.stopTime.toString("HH:mm"));
    bookingObj.insert("Event", booking.event);
    bookingObj.insert("Conflicting", booking.isConflicting ? "true" : "false");
    bookingObj.insert("Index", QString::number(booking.index));

    arr.append(bookingObj);
  }

  writeJsonArrayToFile(arr, bookingsPath);
}

void JsonParser::getAllBookings(QVector<Booking>& outVector) 
{
  outVector.clear();

  QJsonArray arr = readJsonArrayFromFile(bookingsPath);

  if (arr.isEmpty()) return;

  for (const QJsonValue& val : arr) {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    Booking booking = {
      obj.value("Email").toString(),
      QDate::fromString(obj.value("Date").toString(), Qt::ISODate),
      QTime::fromString(obj.value("StartTime").toString("HH:mm")),
      QTime::fromString(obj.value("StopTime").toString("HH:mm")),
      obj.value("Event").toString(),
      obj.value("Conflicting") == "true" ? true : false,
      obj.value("Index").toVariant().toInt()
    };

    // Bookings a week in the past and older are not loaded, and later fizzled
    if (booking.date.addDays(7) > QDate::currentDate())
      outVector.append(booking);
  }
}

void JsonParser::removeBooking(const Booking* booking)
{
  if (!booking) return;

  QJsonArray arr = readJsonArrayFromFile(bookingsPath);

  for (qsizetype i = arr.size() - 1; i >= 0; --i) {
    QJsonValue val = arr[i];

    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if (obj.value("Index").toVariant().toInt() == booking->index)
      arr.removeAt(i);
  }

  writeJsonArrayToFile(arr, bookingsPath); 
}

int JsonParser::availableIndex()
{
  int index = 0;
  QVector<int> occupiedIndices;

  QJsonArray arr = readJsonArrayFromFile(bookingsPath);

  for (const QJsonValue& val : arr) {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    occupiedIndices.append(obj.value("Index").toVariant().toInt());
  }

  while (occupiedIndices.contains(index))
    ++index;

  return index;  
}

void JsonParser::addPreset(const QString& email, const int preset)
{
  QJsonArray arr = readJsonArrayFromFile(presetsPath);

  for (qsizetype i = 0; i < arr.size(); ++i) {
    const QJsonValue& val = arr[i];

    if (!val.isObject())
      continue;

    QJsonObject obj = val.toObject();

    if (!obj.contains("Email") || obj.value("Email").toString() != email)
      continue;

    QJsonArray presetsArray = obj.value("Presets").toArray();

    if (presetsArray.contains(QJsonValue(preset))) return;
    
    presetsArray.append(QJsonValue(preset));
    
    // sort...
    QVector<int> presetsVector;
    
    for (const QJsonValue& val : presetsArray)
      presetsVector.append(val.toVariant().toInt());

    std::sort(presetsVector.begin(), presetsVector.end());

    QJsonArray sortedPresetsArray;

    for (int p : presetsVector)
      sortedPresetsArray.append(QJsonValue(p));
    
    obj["Presets"] = sortedPresetsArray;

    arr[i] = obj;

    writeJsonArrayToFile(arr, presetsPath);

    return;
  }

  QJsonArray presetsArray = {preset};
  QJsonObject presetObj;
  presetObj.insert("Email", email);
  presetObj.insert("Presets", presetsArray);

  arr.append(presetObj);

  writeJsonArrayToFile(arr, presetsPath);
}

void JsonParser::removePreset(const QString& email, const int preset)
{
  QJsonArray arr = readJsonArrayFromFile(presetsPath);

  if (arr.isEmpty()) return;

  for (qsizetype i = 0; i < arr.size(); ++i) {
    const QJsonValue& val = arr[i];

    if (!val.isObject())
      continue;

    QJsonObject obj = val.toObject();

    if (obj.value("Email").toString() == MailHandler::adminEmail && 
        email != MailHandler::adminEmail)
      continue;

    QJsonArray presetsArray = obj.value("Presets").toArray();

    if (presetsArray.isEmpty()) continue;
    
    for (qsizetype j = 0; j < presetsArray.size(); ++j) {
      if (presetsArray[j] == preset)
        presetsArray.removeAt(j);
    }

    obj["Presets"] = presetsArray;

    arr[i] = obj;
  }

  writeJsonArrayToFile(arr, presetsPath);
}

void JsonParser::getPresetsForEmail(const QString& email, QVector<int>& outVector)
{
  QJsonArray arr = readJsonArrayFromFile(presetsPath);

  if (arr.isEmpty())
    return;
  
  outVector.clear();

  for (const QJsonValue& val : arr) {
    if (!val.isObject()) continue;

    QJsonObject obj = val.toObject();

    if ((!obj.contains("Email")) || 
        (obj.value("Email").toString() != email && 
          obj.value("Email").toString() != MailHandler::adminEmail))
      continue;

    QJsonArray presetsArray = obj.value("Presets").toArray();

    for (const QJsonValue& presetVal : presetsArray) {
      outVector.append(presetVal.toVariant().toInt());
    }
  }

  QString presets = email + ": ";

  for (int pr : outVector)
    presets.append(QString::number(pr) + " ");
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

void JsonParser::updatePaths()
{
  bookingsPath = StorageHandler::baseDirectory + "bookings.json";
  presetsPath = StorageHandler::baseDirectory + "presets.json";
}
