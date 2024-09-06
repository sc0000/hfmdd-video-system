#include <QCoreApplication>
#include <QProcess>
#include <QDir>
#include <QDate>
#include <QJsonObject>
#include <QJsonDocument>

#include <obs.hpp>
#include <util/config-file.h>
#include <obs-frontend-api.h>
#include <obs-module.h>

#include "source-record.h"
#include "storage-handler.hpp"
#include "text-handler.hpp"
#include "mail-handler.hpp"

QString MailHandler::nasIP = "";
QString MailHandler::nasPort = "";
QString MailHandler::nasUser = "";
QString MailHandler::nasPassword = "";

QString MailHandler::mailHost = "";
QString MailHandler::mailPort = "";
QString MailHandler::mailUser = "";
QString MailHandler::mailPassword = "";
QString MailHandler::mailSenderAddress = "";

QString MailHandler::adminEmail = "";
QString MailHandler::adminPassword = "";

QString MailHandler::currentEmail = "";
bool MailHandler::mailAddressIsValid = false;
bool MailHandler::isAdmin = false;

QVector<QString> MailHandler::mailSuffices = { "@hfmdd.de", "@mailbox.hfmdd.de", "@gmx.net", "@gmail.com", "@gmail.de" };

bool MailHandler::sendConflictWarnings = true;

void MailHandler::saveCredentials()
{
  char* file = obs_module_config_path("config3.json");

	if (!file) return;

  OBSData savedata = obs_data_create();
	obs_data_release(savedata);

  obs_data_set_string(savedata, "admin_email", adminEmail.toUtf8().constData());
  obs_data_set_string(savedata, "admin_password", adminPassword.toUtf8().constData());

  obs_data_set_string(savedata, "nas_ip", nasIP.toUtf8().constData());
  obs_data_set_string(savedata, "nas_port", nasPort.toUtf8().constData());
  obs_data_set_string(savedata, "nas_user", nasUser.toUtf8().constData());
  obs_data_set_string(savedata, "nas_password", nasPassword.toUtf8().constData());
  obs_data_set_string(savedata, "mail_host", mailHost.toUtf8().constData());
  obs_data_set_string(savedata, "mail_port", mailPort.toUtf8().constData());
  obs_data_set_string(savedata, "mail_user", mailUser.toUtf8().constData());
  obs_data_set_string(savedata, "mail_password", mailPassword.toUtf8().constData());
  obs_data_set_string(savedata, "sender_address", mailSenderAddress.toUtf8().constData());

  if (!obs_data_save_json_safe(savedata, file, "tmp", "bak")) {
		char *path = obs_module_config_path("");

		if (path) {
			os_mkdirs(path);
			bfree(path);
		}

		obs_data_save_json_safe(savedata, file, "tmp", "bak");
  }

	bfree(file);
}

void MailHandler::loadCredentials()
{
  char *file = obs_module_config_path("config3.json");

  if (!file) return;

	OBSData loaddata = obs_data_create_from_json_file_safe(file, "bak");

  bfree(file);

	if (!loaddata) return;
  
	obs_data_release(loaddata);

  adminEmail = obs_data_get_string(loaddata, "admin_email");
  adminPassword = obs_data_get_string(loaddata, "admin_password");

  nasIP = obs_data_get_string(loaddata, "nas_ip");
  nasPort = obs_data_get_string(loaddata, "nas_port");
  nasUser = obs_data_get_string(loaddata, "nas_user");
  nasPassword = obs_data_get_string(loaddata, "nas_password");
  mailHost = obs_data_get_string(loaddata, "mail_host");
  mailPort = obs_data_get_string(loaddata, "mail_port");
  mailUser = obs_data_get_string(loaddata, "mail_user");
  mailPassword = obs_data_get_string(loaddata, "mail_password");
  mailSenderAddress = obs_data_get_string(loaddata, "sender_address");
}

QString MailHandler::sendMail(EMailType mailType, const Booking* booking, 
  const QString& subject, const QString& body)
{
  if (!booking)
    return QString();

  if (mailType == EMailType::BookingConflictWarning && !sendConflictWarnings)
    return QString();

  const QString dllFilePath = QCoreApplication::applicationFilePath();
  const QString dllDir = QFileInfo(dllFilePath).absolutePath();

  const QString scriptPath = QDir::toNativeSeparators(dllDir + "/../../node-scripts/mail-sender.js");

  const QStringList nodeArgs = QStringList() << scriptPath;

  QProcess* nodeProcess = new QProcess();
  nodeProcess->setEnvironment(QProcess::systemEnvironment());
  nodeProcess->setEnvironment(QStringList() << "DEBUG_MODE=false");

  nodeProcess->start("node", nodeArgs);

  if (!nodeProcess->waitForStarted()) 
    return "Process started with error: " + nodeProcess->errorString();

  QJsonObject jsonObj;

  jsonObj["mailHost"] = mailHost;
  jsonObj["mailPort"] = mailPort;
  jsonObj["mailUser"] = mailUser;
  jsonObj["mailPassword"] = mailPassword;
  jsonObj["mailSenderAddress"] = mailSenderAddress;

  QString dateTime = booking->date.toString("ddd MMM dd yyyy\n") + 
    booking->startTime.toString("HH:mm") + " - " +
    booking->stopTime.toString("HH:mm");

  switch (mailType) {
    case EMailType::SendFiles:
      jsonObj["type"] = "send-files";
      jsonObj["receiver"] = booking->email;
      jsonObj["subject"] = TextHandler::getText(ID::MAIL_FILES_SUBJECT) + 
        booking->date.toString("ddd MMM dd yyyy");
      jsonObj["mailBody"] = TextHandler::getText(ID::MAIL_FILES_BODY);

      jsonObj["basePath"] = QString("/team-folders/video/") +
        StorageHandler::baseDirectory.last(StorageHandler::baseDirectory.size() - 3);
      jsonObj["path"] = StorageHandler::outerDirectory + StorageHandler::innerDirectory;

      jsonObj["nasIP"] = nasIP;
      jsonObj["nasPort"] = nasPort;
      jsonObj["nasUser"] = nasUser;
      jsonObj["nasPassword"] = nasPassword;
      break;

    case EMailType::BookingConflictWarning:
      jsonObj["type"] = "booking-conflict-warning";
      jsonObj["receiver"] = adminEmail;
      jsonObj["subject"] = TextHandler::getText(ID::MAIL_CONFLICT_SUBJECT);
      jsonObj["mailBody"] = TextHandler::getText(ID::MAIL_CONFLICT_BODY)
        .arg(dateTime)
        .arg(booking->email)
        .arg(booking->event);
      break;

    case EMailType::AdminEmail:
      jsonObj["type"] = "admin-email";
      jsonObj["receiver"] = booking->email;
      jsonObj["subject"] = subject;
      jsonObj["mailBody"] = body;
      break;

    case EMailType::Default:
      return "Mail type specification invalid. ";
  }

  QJsonDocument jsonDoc(jsonObj);

  nodeProcess->write(jsonDoc.toJson());
  nodeProcess->closeWriteChannel();

  if (!nodeProcess->waitForFinished())
    return "Process finished with error: " + nodeProcess->errorString();

  const QByteArray output = nodeProcess->readAllStandardOutput();
  return QString::fromUtf8(output);
}