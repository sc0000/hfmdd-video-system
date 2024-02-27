#include <obs-frontend-api.h>
#include <QString>
#include <QVector>

class PathManager
{
public:
  static QString baseDirectory;
  static QString outerDirectory;
  static QString innerDirectory;

  static QVector<obs_source_t*> sources;

  static void updateFilterSettings(const char* path);
  static void resetFilterSettings();
};