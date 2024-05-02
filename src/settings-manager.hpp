#include <obs-frontend-api.h>
#include <QString>
#include <QVector>

class SettingsManager
{
public:
  static QString baseDirectory;
  static QString outerDirectory;
  static QString innerDirectory;

  static QVector<obs_source_t*> sources;

  static QString filenameFormatting;
  static QString recFormat;
  static QString qualityPreset;

  static void updateFilterSettings(const char* path);
  static void resetFilterSettings();
  static void save();
  static void load();
  static void setTempPath();
  static void deleteTempFiles();
};