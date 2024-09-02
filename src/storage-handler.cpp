#include <QDir>

#include <obs.hpp>
#include <util/config-file.h>
#include <obs-frontend-api.h>

#include "booking-handler.hpp"
#include "mail-handler.hpp"
#include "storage-handler.hpp"

QString StorageHandler::baseDirectory = "V:/sb-terminal-test/";
QString StorageHandler::outerDirectory = "";
QString StorageHandler::innerDirectory = "";

void StorageHandler::setOuterDirectory()
{
  outerDirectory = BookingHandler::currentBooking->date.toString(Qt::ISODate) + "/";
}

void StorageHandler::setInnerDirectory()
{
  for (const QString& suffix : MailHandler::mailSuffices)
  {
    if (MailHandler::currentEmail.endsWith(suffix))
    {
      innerDirectory = MailHandler::currentEmail.chopped(suffix.length());
      break;
    }
  }
}

const QString StorageHandler::getPath()
{
  return baseDirectory + outerDirectory + innerDirectory;
}

void StorageHandler::setTempPath()
{
  config_t* currentProfile = obs_frontend_get_profile_config();

  if (!currentProfile) return;

  QString sceneFilePath = StorageHandler::baseDirectory + "temp/";
  QDir tempDir = QDir(sceneFilePath);

  if (!tempDir.exists())
    tempDir.mkpath(sceneFilePath);

  const char* c_sceneFilePath = sceneFilePath.toUtf8().constData();

  config_set_string(currentProfile, "SimpleOutput", "FilePath", c_sceneFilePath);
}

void StorageHandler::deleteTempFiles()
{
  QDir tempDir(baseDirectory + "temp/");
  tempDir.setNameFilters(QStringList() << "*.*");
  tempDir.setFilter(QDir::Files);

  for (const QString& file : tempDir.entryList())
    tempDir.remove(file);
}