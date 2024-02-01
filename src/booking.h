#pragma once

#include <QString>
#include <QDateTime>

struct Booking
{
  QString email;
  QTime startTime;
  QTime stopTime;
};