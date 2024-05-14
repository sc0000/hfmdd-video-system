#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

#include "json-parser.hpp"
#include "settings-manager.hpp" // TODO: Rename!
#include "backend.hpp"

QString Backend::currentEmail = "";
const QString Backend::adminEmail = "oliver.fenk@hfmdd.de";
const QString Backend::adminPassword = "pw";
EMode Backend::mode = EMode::Default;
ELanguage Backend::language = ELanguage::English;

QDate Backend::selectedDate = QDate();

Booking Backend::currentBooking = {0};
QVector<Booking> Backend::loadedBookings = {};
QVector<Booking> Backend::bookingsOnSelectedDate = {};

void Backend::updateConflictingBookings(const QDate& date)
{
  updateBookingsOnSelectedDate(date);

  for (Booking& b : bookingsOnSelectedDate) {
    if (bookingsAreConflicting(currentBooking, b)) {
      currentBooking.isConflicting = true;
      b.isConflicting = true;
    }

    else {
      currentBooking.isConflicting = false;
      b.isConflicting = false;
    }

    // ??? b.date = date;

    JsonParser::updateBooking(b);
  }
}

bool Backend::bookingsAreConflicting(const Booking& b0, const Booking& b1)
{
  if (b0.index == b1.index) return false;

  if ((b0.startTime >= b1.startTime && b0.startTime < b1.stopTime) ||
      (b0.stopTime > b1.startTime && b0.stopTime <= b1.stopTime))
    return true;

  return false; 
}

void Backend::updateBookingsOnSelectedDate(const QDate& date)
{
  JsonParser::getBookingsOnDate(date, bookingsOnSelectedDate);
}

void Backend::loadBookings()
{
  if (currentEmail == adminEmail)
    JsonParser::getAllBookings(loadedBookings);

  else 
    JsonParser::getBookingsForEmail(currentEmail, loadedBookings);

  sortBookings();
}

void Backend::sortBookings()
{
  qsizetype size = loadedBookings.size();

  for (qsizetype i = 0; i < size; ++i) {
    bool flag = false;

    for (qsizetype j = 0; j < size - 1 - i; ++j) {
      QString dateTime0 = loadedBookings[j].date.toString(Qt::ISODate) + "_" +
        loadedBookings[j].startTime.toString("HH:mm");

      QString dateTime1 = loadedBookings[j + 1].date.toString(Qt::ISODate) + "_" +
        loadedBookings[j + 1].startTime.toString("HH:mm");

      if (dateTime0 > dateTime1) {
        Booking tmp = loadedBookings[j];
        loadedBookings[j] = loadedBookings[j + 1];
        loadedBookings[j + 1] = tmp;

        flag = true;
      }
    }

    if (flag == 0) break;
  }
}

void Backend::reevaluateConflicts()
{
  // load all bookings (locally)
  QVector<Booking> allBookings;
  JsonParser::getAllBookings(allBookings);

  for (qsizetype i = 0; i < allBookings.size(); ++i) {
    bool isConflicting = false;

    for (qsizetype j = 0; j < allBookings.size(); ++j) {
      if (bookingsAreConflicting(allBookings[i], allBookings[j])) 
        isConflicting = true; 
    }

    allBookings[i].isConflicting = isConflicting;
  }
}

void Backend::roundTime(QTime& time)
{
  int minutes = time.minute();
  int remainder = minutes % 10;
  int roundedMinutes = minutes - remainder;

  if (remainder > 4)
    roundedMinutes += 10;

  if (roundedMinutes >= 60) {
    roundedMinutes = 0;
    int hour = time.hour();

    if (hour == 23) 
      time = QTime(0, 0);

    else 
      time = QTime(hour + 1, 0);
  }

  else
    time = QTime(time.hour(), roundedMinutes);
}

QString Backend::sendFiles(const Booking& booking)
{
  const QString dllFilePath = QCoreApplication::applicationFilePath();
  const QString dllDir = QFileInfo(dllFilePath).absolutePath();

  const QString scriptPath = QDir::toNativeSeparators(dllDir + "/../../node-scripts/file-sender.js");

  const QStringList nodeArgs = QStringList() << scriptPath;

  QProcess* nodeProcess = new QProcess();
  nodeProcess->setEnvironment(QProcess::systemEnvironment());
  nodeProcess->setEnvironment(QStringList() << "DEBUG_MODE=false");

  nodeProcess->start("node", nodeArgs);

  if (!nodeProcess->waitForStarted()) 
    return "Process started with error: " + nodeProcess->errorString();

  const QString& baseDir = SettingsManager::baseDirectory;

  const QString apiBasePath = 
    QString("/team-folders/video/") + baseDir.last(baseDir.size() - 3);

  QJsonObject jsonObj;
  jsonObj["basePath"] = apiBasePath;
  jsonObj["path"] = SettingsManager::outerDirectory + SettingsManager::innerDirectory;
  jsonObj["receiver"] = booking.email;
  jsonObj["subject"] = "HfMDD Concert Hall Recordings " + booking.date.toString("ddd MMM dd yyyy");

  QJsonDocument jsonDoc(jsonObj);

  nodeProcess->write(jsonDoc.toJson());
  nodeProcess->closeWriteChannel();

  if (!nodeProcess->waitForFinished())
    return "Process finished with error: " + nodeProcess->errorString();

  const QByteArray output = nodeProcess->readAllStandardOutput();
  return QString::fromUtf8(output);
}