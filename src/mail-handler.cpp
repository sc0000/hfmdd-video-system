/* Sending mails to users and admin
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

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
#include "widgets.hpp"
#include "message-dialog.hpp"
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

QVector<QString> MailHandler::mailSuffices = { "@hfmdd.de", "@mailbox.hfmdd.de", "admin.default" };

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

void MailHandler::sendMail(EMailType mailType, const Booking* booking, 
  const QString& subject, const QString& body)
{
  if (!booking) 
    return;

  if (mailType == EMailType::BookingConflictWarning && !sendConflictWarnings)
    return;

  const QString dllFilePath = QCoreApplication::applicationFilePath();
  const QString dllDir = QFileInfo(dllFilePath).absolutePath();

  const QString scriptPath = QDir::toNativeSeparators(dllDir + "/../../node-scripts/mail-sender.js");

  const QStringList nodeArgs = QStringList() << scriptPath;

  QProcess* nodeProcess = new QProcess();
  nodeProcess->setEnvironment(QProcess::systemEnvironment());
  nodeProcess->setEnvironment(QStringList() << "DEBUG_MODE=false");

  QJsonObject jsonObj;

  jsonObj["mailHost"] = mailHost;
  jsonObj["mailPort"] = mailPort;
  jsonObj["mailUser"] = mailUser;
  jsonObj["mailPassword"] = mailPassword;
  jsonObj["mailSenderAddress"] = mailSenderAddress;

  const QString& format = TextHandler::getText(ID::DATE_FORMAT);

  switch (mailType) {
    case EMailType::SendFiles:
      jsonObj["type"] = "send-files";
      jsonObj["receiver"] = booking->email;
      jsonObj["subject"] = TextHandler::getText(ID::MAIL_FILES_SUBJECT) + 
        booking->date.toString(TextHandler::getText(ID::DATE_FORMAT));
      jsonObj["mailBody"] = TextHandler::getText(ID::MAIL_FILES_BODY)
        .arg(adminEmail);

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
      // ! German version of this mail hard coded for now...
      jsonObj["subject"] = TextHandler::getTextGerman(ID::MAIL_CONFLICT_SUBJECT);
      jsonObj["mailBody"] = TextHandler::getTextGerman(ID::MAIL_CONFLICT_BODY)
        .arg(booking->email)
        .arg(booking->event)
        .arg(TextHandler::locale.toString(booking->date, format))
        .arg(booking->startTime.toString("HH:mm"))
        .arg(booking->stopTime.toString("HH:mm"));
      break;

    case EMailType::AdminEmail:
      jsonObj["type"] = "admin-email";
      jsonObj["receiver"] = booking->email;
      jsonObj["subject"] = subject;
      jsonObj["mailBody"] = body;
      break;

    case EMailType::Default:
      break;
  }

  QJsonDocument jsonDoc(jsonObj);
  QByteArray jsonData = jsonDoc.toJson();

  QObject::connect(nodeProcess, &QProcess::started, [nodeProcess, jsonData]() {
        nodeProcess->write(jsonData);
        nodeProcess->closeWriteChannel();
    }
  );

  QObject::connect(nodeProcess, &QProcess::finished, [nodeProcess](int exitCode, QProcess::ExitStatus exitStatus) {
      nodeProcess->deleteLater();
    }
  );

  nodeProcess->start("node", nodeArgs);
}
