#pragma once

#include <QWidget>
#include <QString>

class Globals
{
public:
  static QString currentEmail;
  const static QString oliversEmail;

  static void centerFullScreenWidget(QWidget* masterWidget);
};