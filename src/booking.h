#pragma once

#include <QString>
#include <QDateTime>

struct Booking
{
  QString email;
  QDate date;
  QTime startTime;
  QTime stopTime;
  QString event;
  bool isConflicting;
  int index;
};

