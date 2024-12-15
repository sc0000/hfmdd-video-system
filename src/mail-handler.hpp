/* Sending mails to users and admin
 *
 * Copyright 2024 Sebastian Cyliax <sebastiancyliax@gmx.net>
 *
 * SPDX-License-Identifier: GPLv2
 */

#pragma once

#include "booking-handler.hpp"

enum class EMailType
{
  SendFiles,
  BookingConflictWarning,
  AdminEmail,
  Default
};

class MailHandler 
{
public:
  static QString nasIP;
  static QString nasPort;
  static QString nasUser;
  static QString nasPassword;
  
  static QString mailHost;
  static QString mailPort;
  static QString mailUser;
  static QString mailPassword;
  static QString mailSenderAddress;

  static QString adminEmail;
  static QString adminPassword;

  static QString currentEmail;
  static bool mailAddressIsValid;
  static bool isAdmin;

  static QVector<QString> mailSuffices;

  static bool sendConflictWarnings;

  static void sendMail(EMailType mailType, const Booking* booking, const QString& subject = "", const QString& body = "");

  static void saveCredentials();
  static void loadCredentials(); 
};
