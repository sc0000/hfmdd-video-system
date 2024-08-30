#pragma once

#include <QString>

class StorageHandler
{
public:
  static QString baseDirectory;
  static QString outerDirectory;
  static QString innerDirectory;

  static void setOuterDirectory();
  static void setInnerDirectory();

  static const QString getPath();
  
  static void setTempPath();
  static void deleteTempFiles();
};