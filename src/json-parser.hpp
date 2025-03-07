/* Reading from and writing to JSON files (bookings and presets)
 * 
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include "booking-handler.hpp"

class JsonParser
{
public:

  static void addBooking(const Booking* booking);
  static void updateBooking(const Booking* booking);
  static void updateAllBookings(const QVector<Booking>& allBookings);
  static void getAllBookings(QVector<Booking>& outVector);
  static void removeBooking(const Booking* booking);
  static int availableIndex();

  static void addPreset(const QString& email, const int preset);
  static void removePreset(const QString& email, const int preset);
  static void getPresetsForEmail(const QString& email, QVector<int>& outVector);

  static QJsonArray readJsonArrayFromFile(const QString& path);
  static void writeJsonArrayToFile(const QJsonArray& arr, const QString& path);

  inline static void setBookingsPath(const QString& newPath) { bookingsPath = newPath + "bookings.json"; }
  inline static void setPresetsPath(const QString& newPath) { presetsPath = newPath + "bookings.json"; }

  static void updatePaths();

  static QString bookingsPath;
  static QString presetsPath;
};
