#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QProcess>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

#include "globals.hpp"
#include "path-manager.hpp"
#include "message-dialog.hpp"

#include "file-sender.hpp"

QString FileSender::sendFiles(const Booking& booking)
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

  const QString& baseDir = PathManager::baseDirectory;

  const QString apiBasePath = 
    QString("/team-folders/video/") + baseDir.last(baseDir.size() - 3);

  QJsonObject jsonObj;
  jsonObj["basePath"] = apiBasePath;
  jsonObj["path"] = PathManager::outerDirectory + PathManager::innerDirectory;
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