#pragma once

#include <QWidget>
#include <QString>

enum class EMode
{
  QuickMode,
  BookMode,
  Default
};

class Globals
{
public:
  static QString currentEmail;
  const static QString oliversEmail;
  static EMode mode;

  static void centerFullScreenWidget(QWidget* masterWidget);
};