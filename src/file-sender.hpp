#pragma once

#include <QString>
#include "booking.h"

class FileSender
{
public:
  static QString sendFiles(const Booking& booking);
};