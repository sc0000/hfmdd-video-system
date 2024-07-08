#include <obs-frontend-api.h>
#include <QString>
#include <QVector>

class SettingsManager
{
public:
  static QString baseDirectory;
  static QString outerDirectory;
  static QString innerDirectory;

  static QVector<QString> mailSuffices;

  static QVector<obs_source_t*> sources;

  static QString filenameFormatting;
  static QString recFormat;
  static QString qualityPreset;

  static QString nasIP;
  static QString nasPort;
  static QString nasUser;
  static QString nasPassword;
  
  static QString mailHost;
  static QString mailUser;
  static QString mailPassword;
  static QString mailSenderAddress;

  static void setOuterDirectory();
  static void setInnerDirectory();
  static void updateFilterSettings(const char* path);
  static void resetFilterSettings();
  static void saveSettings();
  static void saveCredentials();
  static void loadSettings();
  static void loadCredentials();
  static void setTempPath();
  static void deleteTempFiles();
};